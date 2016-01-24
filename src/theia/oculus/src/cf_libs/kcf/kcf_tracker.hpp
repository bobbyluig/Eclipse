#ifndef KCF_TRACKER_HPP_
#define KCF_TRACKER_HPP_

#include <boost/python.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/traits.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <algorithm>

#include "cv_ext.hpp"
#include "feature_channels.hpp"
#include "gradientMex.hpp"
#include "mat_consts.hpp"
#include "math_helper.hpp"
#include "scale_estimator.hpp"
#include "psr.hpp"
#include "gil.hpp"

using namespace boost::python;

namespace cf_tracking
{
    struct KcfParameters
    {
        double padding = 1.7;
        double lambda = 0.0001;
        double outputSigmaFactor = 0.05;
        double votScaleStep = 1.05;
        double votScaleWeight = 0.95;
        int templateSize = 100;
        double interpFactor = 0.012;
        double kernelSigma = 0.6;
        int cellSize = 4;
        int pixelPadding = 0;

        bool enableTrackingLossDetection = false;
        double psrThreshold = 13.5;
        int psrPeakDel = 1;

        bool useVotScaleEstimation = false;
        bool useDsstScaleEstimation = true;
        double scaleSigmaFactor = static_cast<double>(0.25);
        double scaleEstimatorStep = static_cast<double>(1.02);
        double scaleLambda = static_cast<double>(0.01);
        int scaleCellSize = 4;
        int numberOfScales = 33;

        // testing
        int resizeType = cv::INTER_LINEAR;
        bool useFhogTranspose = false;
		double minArea = static_cast<double>(10);
		double maxAreaFactor = static_cast<double>(0.8);
		int nScalesVot = 3;
		double VotMinScaleFactor = static_cast<double>(0.01);
		double VotMaxScaleFactor = static_cast<double>(40);
		bool useCcs = true;
    };

    class KcfTracker
    {
    public:
        static const int NUM_FEATURE_CHANNELS = 31;
        typedef double T; // set precision here: double or float
        static const int CV_TYPE = cv::DataType<T>::type;
        typedef cv::Size_<T> Size;
        typedef FhogFeatureChannels<T> FFC;
        typedef mat_consts::constants<T> consts;
        typedef cv::Point_<T> Point;
        typedef cv::Rect_<T> Rect;
		typedef boost::python::tuple Tuple;

        KcfTracker(KcfParameters paras)
            : _isInitialized(false),
            _PADDING(static_cast<T>(paras.padding)),
            _LAMBDA(static_cast<T>(paras.lambda)),
            _OUTPUT_SIGMA_FACTOR(static_cast<T>(paras.outputSigmaFactor)),
            _SCALE_STEP(static_cast<T>(paras.votScaleStep)),
            _SCALE_WEIGHT(static_cast<T>(paras.votScaleWeight)),
            _TEMPLATE_SIZE(paras.templateSize),
            _INTERP_FACTOR(static_cast<T>(paras.interpFactor)),
            _KERNEL_SIGMA(static_cast<T>(paras.kernelSigma)),
            _CELL_SIZE(paras.cellSize),
            _PIXEL_PADDING(paras.pixelPadding),
            _N_SCALES_VOT(paras.nScalesVot),
            _USE_VOT_SCALE_ESTIMATION(paras.useVotScaleEstimation),
            _ENABLE_TRACKING_LOSS_DETECTION(paras.enableTrackingLossDetection),
            _PSR_THRESHOLD(static_cast<T>(paras.psrThreshold)),
            _PSR_PEAK_DEL(paras.psrPeakDel),
            _MIN_AREA(static_cast<T>(paras.minArea)),
            _MAX_AREA_FACTOR(static_cast<T>(paras.maxAreaFactor)),
            _RESIZE_TYPE(paras.resizeType),
            _USE_CCS(paras.useCcs),
			_VOT_MIN_SCALE_FACTOR(paras.VotMinScaleFactor),
			_VOT_MAX_SCALE_FACTOR(paras.VotMaxScaleFactor),
            _scaleEstimator(0)
        {
            correlate = &KcfTracker::gaussianCorrelation;

            if (paras.useDsstScaleEstimation)
            {
                ScaleEstimatorParas<T> sp;
                sp.scaleCellSize = paras.scaleCellSize;
                sp.scaleStep = static_cast<T>(paras.scaleEstimatorStep);
                sp.numberOfScales = paras.numberOfScales;
                sp.scaleSigmaFactor = static_cast<T>(paras.scaleSigmaFactor);
                sp.lambda = static_cast<T>(paras.scaleLambda);
                sp.learningRate = static_cast<T>(paras.interpFactor);
                sp.useFhogTranspose = paras.useFhogTranspose;
                _scaleEstimator = new ScaleEstimator<T>(sp);
            }

            // init dft
            cv::Mat initDft = (cv::Mat_<T>(1, 1) << 1);
            dft(initDft, initDft);

            if (paras.useFhogTranspose)
                cvFhog = &piotr::cvFhogT < T, FFC > ;
            else
                cvFhog = &piotr::cvFhog < T, FFC > ;

        }

        virtual ~KcfTracker()
        {
            delete _scaleEstimator;
        }

		bool reinit(const cv::Mat& image, Tuple boundingBox)
		{
			releaseGIL unlock;

			Rect bb = Rect(
				extract<T>(boundingBox[0]),
				extract<T>(boundingBox[1]),
				extract<T>(boundingBox[2]),
				extract<T>(boundingBox[3])
				);

			return reinit_(image, bb);
		}

		bool update(const cv::Mat& image)
		{
			releaseGIL unlock;

			Rect bb = Rect(
				position[0],
				position[1],
				position[2],
				position[3]
				);

			if (update_(image, bb) == false)
				return false;

			position[0] = static_cast<T>(bb.x);
			position[1] = static_cast<T>(bb.y);
			position[2] = static_cast<T>(bb.width);
			position[3] = static_cast<T>(bb.height);
			return true;
		}

		bool updateAt(const cv::Mat& image, Tuple boundingBox)
		{
			releaseGIL unlock;

			bool isValid = false;

			Rect bb = Rect(
				extract<T>(boundingBox[0]),
				extract<T>(boundingBox[1]),
				extract<T>(boundingBox[2]),
				extract<T>(boundingBox[3])
				);

			isValid = updateAt_(image, bb);

			position[0] = static_cast<T>(bb.x);
			position[1] = static_cast<T>(bb.y);
			position[2] = static_cast<T>(bb.width);
			position[3] = static_cast<T>(bb.height);

			return isValid;
		}

		Tuple getBoundingBox()
		{
			return boost::python::make_tuple(position[0], position[1], position[2], position[3]);
		}

		Tuple getCenter()
		{
			return boost::python::make_tuple(position[0] + position[2] / 2, position[1] + position[3] / 2);
		}

    private:
        bool reinit_(const cv::Mat& image, Rect& boundingBox)
        {
            if (boundingBox.width < 1 || boundingBox.height < 1)
                return false;

            _lastBoundingBox = boundingBox;
            _pos.x = boundingBox.x + boundingBox.width * consts::c0_5;
            _pos.y = boundingBox.y + boundingBox.height * consts::c0_5;

            // original target size for scale estimation
            Size targetSize = Size(boundingBox.width, boundingBox.height);

            _targetSize = targetSize;
            T targetPadding = _PADDING * sqrt(_targetSize.width * _targetSize.height);
            Size templateSz = Size(floor(_targetSize.width + targetPadding),
                floor(_targetSize.height + targetPadding));

            if (templateSz.height > templateSz.width)
                _scale = templateSz.height / _TEMPLATE_SIZE;
            else
                _scale = templateSz.width / _TEMPLATE_SIZE;

            _templateScaleFactor = 1 / _scale;
            _templateSz = Size(floor(templateSz.width / _scale), floor(templateSz.height / _scale));

            _targetSize = Size(_targetSize.width / _scale, _targetSize.height / _scale);

            T outputSigma = sqrt(_templateSz.area() / ((1 + _PADDING) * (1 + _PADDING)))
                * _OUTPUT_SIGMA_FACTOR / _CELL_SIZE;
            Size templateSzByCells = Size(floor((_templateSz.width - _PIXEL_PADDING) / _CELL_SIZE),
                floor((_templateSz.height - _PIXEL_PADDING) / _CELL_SIZE));

            _y = gaussianShapedLabelsShifted2D(outputSigma, templateSzByCells);

            if (_USE_CCS)
                dft(_y, _yf);
            else
                dft(_y, _yf, cv::DFT_COMPLEX_OUTPUT);

            cv::Mat cosWindowX;
            cv::Mat cosWindowY;
            cosWindowY = hanningWindow<T>(_yf.rows);
            cosWindowX = hanningWindow<T>(_yf.cols);
            _cosWindow = cosWindowY * cosWindowX.t();

            cv::Mat numeratorf;
            cv::Mat denominatorf;
            std::shared_ptr<FFC> xf(0);

            if (_scaleEstimator == 0 && _USE_VOT_SCALE_ESTIMATION)
            {
                cv::Mat colScales = numberToColVector<T>(_N_SCALES_VOT);
                T scaleHalf = static_cast<T>(ceil(_N_SCALES_VOT / 2.0));
                cv::Mat ss = colScales - scaleHalf;

                _scaleFactors = pow<T, T>(_SCALE_STEP, ss);
            }

            if (getTrainingData(image, numeratorf, denominatorf, xf) == false)
                return false;

            cv::Mat alphaf;

            if (_USE_CCS)
                divSpectrums(numeratorf, denominatorf, alphaf, 0, false);
            else
                divideSpectrumsNoCcs<T>(numeratorf, denominatorf, alphaf);

            _modelNumeratorf = numeratorf;
            _modelDenominatorf = denominatorf;
            _modelAlphaf = alphaf;
            _modelXf = xf;

            if (_scaleEstimator)
            {
                if (_scaleEstimator->reinit(image, _pos, targetSize,
                    static_cast<float>(_scale * _templateScaleFactor)) == false)
                    return false;
            }

            _isInitialized = true;
            return true;
        }

        bool getTrainingData(const cv::Mat& image, cv::Mat& numeratorf,
            cv::Mat& denominatorf, std::shared_ptr<FFC>& xf)
        {
            std::shared_ptr<FFC> features(0);

            if (getFeatures(image, _pos, _scale, features) == false)
                return false;

            if (_USE_CCS)
                xf = FFC::dftFeatures(features);
            else
                xf = FFC::dftFeatures(features, cv::DFT_COMPLEX_OUTPUT);

            cv::Mat kf = (this->*correlate)(xf, xf);

            cv::Mat kfLambda;

            if (_USE_CCS)
                kfLambda = addRealToSpectrum<T>(_LAMBDA, kf);
            else
                kfLambda = kf + _LAMBDA;

            mulSpectrums(_yf, kf, numeratorf, 0);
            mulSpectrums(kf, kfLambda, denominatorf, 0);

            return true;
        }

        cv::Mat gaussianCorrelation(const std::shared_ptr<FFC>& xf, const std::shared_ptr<FFC>& yf) const
        {
            // TODO: optimization: squaredNormFeatures, mulSpectrumsFeatrues, sumFeatures
            T xx, yy;
            if (_USE_CCS)
            {
                xx = FFC::squaredNormFeaturesCcs(xf);

                // don't recalculate norm if xf == yf
                yy = xx;

                if (xf != yf)
                    yy = FFC::squaredNormFeaturesCcs(yf);
            }
            else
            {
                xx = FFC::squaredNormFeaturesNoCcs(xf);

                // don't recalculate norm if xf == yf
                yy = xx;

                if (xf != yf)
                    yy = FFC::squaredNormFeaturesNoCcs(yf);
            }

            std::shared_ptr<FFC> xyf = FFC::mulSpectrumsFeatures(xf, yf, true);
            std::shared_ptr<FFC> realXy = FFC::idftFeatures(xyf);
            cv::Mat xy = FFC::sumFeatures(realXy);

            T numel = static_cast<T>(xf->channels[0].total() * NUM_FEATURE_CHANNELS);
            calcGaussianTerm(xy, numel, xx, yy);
            cv::Mat kf;

            if (_USE_CCS)
                dft(xy, kf);
            else
                dft(xy, kf, cv::DFT_COMPLEX_OUTPUT);

            return kf;
        }

        void calcGaussianTerm(cv::Mat& xy, T numel, T xx, T yy) const
        {
            int width = xy.cols;
            int height = xy.rows;

            // http://docs.opencv.org/doc/tutorials/core/how_to_scan_images/how_to_scan_images.html#the-efficient-way
            // TODO: this mat is always continuous; remove non-continuous handling
            // TODO: numel division can be done outside the loop
            if (xy.isContinuous())
            {
                width *= height;
                height = 1;
            }

            int row = 0, col = 0;
            T* xyd = 0;

            const T summands = xx + yy;
            const T fraction = -1 / (_KERNEL_SIGMA * _KERNEL_SIGMA);

            for (row = 0; row < height; ++row)
            {
                xyd = xy.ptr<T>(row);

                for (col = 0; col < width; ++col)
                {
                    xyd[col] = (summands - 2 * xyd[col]) / numel;

                    if (xyd[col] < 0)
                        xyd[col] = 0;

                    xyd[col] *= fraction;
                    xyd[col] = exp(xyd[col]);
                }
            }
        }

        bool getFeatures(const cv::Mat& image, const Point& pos,
            const T scale, std::shared_ptr<FFC>& features) const
        {
            cv::Mat patch;
            Size patchSize = _templateSz * scale;

            if (getSubWindow<T>(image, patch, patchSize, pos) == false)
                return false;

            cv::Mat patchResized;
            resize(patch, patchResized, _templateSz, 0, 0, _RESIZE_TYPE);

            cv::Mat patchResizedFloat;
            patchResized.convertTo(patchResizedFloat, CV_32FC(3));

            patchResizedFloat *= 0.003921568627451; // patchResizedFloat /= 255;

            features.reset(new FFC());
            piotr::cvFhog<T, FFC>(patchResizedFloat, features, _CELL_SIZE);
            FFC::mulFeatures(features, _cosWindow);

            return true;
        }

        bool update_(const cv::Mat& image, Rect& boundingBox)
        {
            return updateAtScalePos(image, _pos, _scale, boundingBox);
        }

        bool updateAt_(const cv::Mat& image, Rect& boundingBox)
        {
            bool isValid = false;
            T scale = 0;
            Point pos(boundingBox.x + boundingBox.width * consts::c0_5,
                boundingBox.y + boundingBox.height * consts::c0_5);

            // caller's box may have a different aspect ratio
            // compared to the _targetSize; use the larger side
            // to calculate scale
            if (boundingBox.width > boundingBox.height)
                scale = boundingBox.width / _targetSize.width;
            else
                scale = boundingBox.height / _targetSize.height;

            isValid = updateAtScalePos(image, pos, scale, boundingBox);
            return isValid;
        }

        bool updateAtScalePos(const cv::Mat& image, const Point& oldPos, const T oldScale,
            Rect& boundingBox)
        {
            ++_frameIdx;

            if (!_isInitialized)
                return false;

            T newScale = oldScale;
            Point newPos = oldPos;
            cv::Point2i maxResponseIdx;
            cv::Mat response;

            // in case of error return the last box
            boundingBox = _lastBoundingBox;

            if (detectModel(image, response, maxResponseIdx, newPos, newScale) == false)
                return false;

            // calc new box
            Rect tempBoundingBox;
            tempBoundingBox.width = newScale * _targetSize.width;
            tempBoundingBox.height = newScale * _targetSize.height;
            tempBoundingBox.x = newPos.x - tempBoundingBox.width / 2;
            tempBoundingBox.y = newPos.y - tempBoundingBox.height / 2;

            if (_ENABLE_TRACKING_LOSS_DETECTION)
            {
                if (evalReponse(image, response, maxResponseIdx,
                    tempBoundingBox) == false)
                    return false;
            }

            if (updateModel(image, newPos, newScale) == false)
                return false;

            boundingBox &= Rect(0, 0, static_cast<T>(image.cols), static_cast<T>(image.rows));
            boundingBox = tempBoundingBox;
            _lastBoundingBox = tempBoundingBox;
            return true;
        }

        bool evalReponse(const cv::Mat &image, const cv::Mat& response,
            const cv::Point2i& maxResponseIdx,
            const Rect& tempBoundingBox) const
        {
            T peakValue = 0;
            T psrClamped = calcPsr(response, maxResponseIdx, _PSR_PEAK_DEL, peakValue);

            if (psrClamped < _PSR_THRESHOLD)
                return false;

            // check if we are out of image, too small or too large
            Rect imageRect(Point(0, 0), image.size());
            Rect intersection = imageRect & tempBoundingBox;
            double  bbArea = tempBoundingBox.area();
            double areaThreshold = _MAX_AREA_FACTOR * imageRect.area();
            double intersectDiff = std::abs(bbArea - intersection.area());

            if (intersectDiff > 0.01 || bbArea < _MIN_AREA
                || bbArea > areaThreshold)
                return false;

            return true;
        }

        bool detectModel(const cv::Mat& image, cv::Mat& response, cv::Point2i& maxResponseIdx,
            Point& newPos, T& newScale) const
        {
            double newMaxResponse;

            if (_scaleEstimator || !_USE_VOT_SCALE_ESTIMATION)
            {
                if (getResponse(image, newPos,
                    newScale, response, newMaxResponse,
                    maxResponseIdx) == false)
                    return false;
            }
            else
            {
                if (detectScales(image, newPos,
                    response, maxResponseIdx, newScale) == false)
                    return false;
            }

            cv::Point_<T> subDelta = subPixelDelta<T>(response, maxResponseIdx);
            if (subDelta.y >= response.rows / 2)
                subDelta.y -= response.rows;

            if (subDelta.x >= response.cols / 2)
                subDelta.x -= response.cols;

            T posDeltaX = _CELL_SIZE * subDelta.x;
            T posDeltaY = _CELL_SIZE * subDelta.y;

            if (_USE_VOT_SCALE_ESTIMATION)
            {
                newPos.x += newScale * posDeltaX;
                newPos.y += newScale * posDeltaY;
            }
            else
            {
                newPos.x += _scale * posDeltaX;
                newPos.y += _scale * posDeltaY;
            }

            if (_scaleEstimator)
            {
                //find scale
                T tempScale = newScale * _templateScaleFactor;

                if (_scaleEstimator->detectScale(image, newPos,
                    tempScale) == false)
                    return false;

                newScale = tempScale / _templateScaleFactor;
            }

            // shift max response to the middle to ease PSR extraction
            cv::Point2f delta(static_cast<float>(floor(_yf.cols * 0.5) + 1),
                static_cast<float>(floor(_yf.rows * 0.5) + 1));

            shift(response, response, delta, cv::BORDER_WRAP);

            maxResponseIdx.x = mod(static_cast<int>(delta.x) + maxResponseIdx.x, _yf.cols);
            maxResponseIdx.y = mod(static_cast<int>(delta.y) + maxResponseIdx.y, _yf.rows);

            return true;
        }

        bool updateModel(const cv::Mat& image, const Point& newPos,
            const T& newScale)
        {
            _scale = newScale;
            _pos = newPos;
            cv::Mat numerator;
            cv::Mat denominator;
            std::shared_ptr<FFC> xf(0);

            if (getTrainingData(image, numerator, denominator, xf) == false)
                return false;

            _modelNumeratorf = (1 - _INTERP_FACTOR) * _modelNumeratorf + _INTERP_FACTOR * numerator;
            _modelDenominatorf = (1 - _INTERP_FACTOR) * _modelDenominatorf + _INTERP_FACTOR * denominator;
            FFC::mulValueFeatures(_modelXf, (1 - _INTERP_FACTOR));
            FFC::mulValueFeatures(xf, _INTERP_FACTOR);
            FFC::addFeatures(_modelXf, xf);
            cv::Mat alphaf;

            if (_USE_CCS)
                divSpectrums(_modelNumeratorf, _modelDenominatorf, alphaf);
            else
                divideSpectrumsNoCcs<T>(_modelNumeratorf, _modelDenominatorf, alphaf);

            _modelAlphaf = alphaf;

            if (_scaleEstimator)
            {
                if (_scaleEstimator->updateScale(image, newPos, newScale * _templateScaleFactor) == false)
                    return false;
            }

            return true;
        }

        bool detectScales(const cv::Mat& image, const Point& pos,
            cv::Mat& response, cv::Point2i& maxResponseIdx, T& scale) const
        {
            double maxResponse = 0;

            cv::Mat* newResponses = new cv::Mat[_N_SCALES_VOT];
            cv::Point2i* newMaxIdxs = new cv::Point2i[_N_SCALES_VOT];
            double* newMaxResponses = new double[_N_SCALES_VOT]{};
            bool* validResults = new bool[_N_SCALES_VOT]{};
            T* newScales = new T[_N_SCALES_VOT]{};

            for (int i = 0; i < _N_SCALES_VOT; ++i)
                newScales[i] = scale * _scaleFactors.at<T>(0, i);

#pragma omp parallel for
            for (int i = 0; i < _N_SCALES_VOT; ++i)
                validResults[i] = getResponse(image, pos,
                newScales[i], newResponses[i], newMaxResponses[i], newMaxIdxs[i]);

            bool validFound = false;

            for (int i = 0; i < _N_SCALES_VOT; ++i)
                validFound |= validResults[i];

            if (validFound == false)
                return false;

            int bestIdx = static_cast<int>(floor(_N_SCALES_VOT / 2.0));
            maxResponse = newMaxResponses[bestIdx];

            for (int i = 0; i < _N_SCALES_VOT; ++i)
            {
                if (validResults[i] &&
                    newMaxResponses[i] * _SCALE_WEIGHT > maxResponse)
                {
                    maxResponse = newMaxResponses[i];
                    bestIdx = i;
                }
            }

            response = newResponses[bestIdx];
            maxResponseIdx = newMaxIdxs[bestIdx];
            scale = newScales[bestIdx];
            scale = std::max(_VOT_MIN_SCALE_FACTOR, scale);
            scale = std::min(_VOT_MAX_SCALE_FACTOR, scale);

            delete[] newResponses;
            delete[] newMaxIdxs;
            delete[] newMaxResponses;
            delete[] validResults;
            delete[] newScales;

            return true;
        }

        bool getResponse(const cv::Mat& image, const Point& pos,
            T scale, cv::Mat &newResponse, double& newMaxResponse,
            cv::Point2i& newMaxIdx) const
        {
            if (detect(image, pos, scale, newResponse) == false)
                return false;

            minMaxLoc(newResponse, 0, &newMaxResponse, 0, &newMaxIdx);

            return true;
        }

        bool detect(const cv::Mat& image, const Point& pos,
            T scale, cv::Mat& response) const
        {
            std::shared_ptr<FFC> features(0);

            if (getFeatures(image, pos, scale, features) == false)
                return false;

            std::shared_ptr<FFC> zf;
            if (_USE_CCS)
                zf = FFC::dftFeatures(features);
            else
                zf = FFC::dftFeatures(features, cv::DFT_COMPLEX_OUTPUT);

            cv::Mat kzf = (this->*correlate)(zf, _modelXf);
            cv::Mat responsef;
            mulSpectrums(_modelAlphaf, kzf, responsef, 0, false);
            idft(responsef, response, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
            return true;
        }

    private:
        KcfTracker& operator=(const KcfTracker&)
        {}

    private:
        typedef cv::Mat(KcfTracker::*correlatePtr)(const std::shared_ptr<FFC>&,
            const std::shared_ptr<FFC>&) const;
        correlatePtr correlate = 0;

        typedef void(*cvFhogPtr)
            (const cv::Mat& img, std::shared_ptr<FFC>& cvFeatures, int binSize, int fhogChannelsToCopy);
        cvFhogPtr cvFhog = 0;

        cv::Mat _cosWindow;
        cv::Mat _y;
        std::shared_ptr<FFC> _modelXf = 0;
        cv::Mat _modelNumeratorf;
        cv::Mat _modelDenominatorf;
        cv::Mat _modelAlphaf;
        cv::Mat _yf;
        cv::Mat _scaleFactors;
        Rect _lastBoundingBox;
        Point _pos;
        Size _targetSize;
        Size _templateSz;
        T _scale;
        T _templateScaleFactor;
        int _frameIdx = 1;
        bool _isInitialized;
        ScaleEstimator<T>* _scaleEstimator;

        const T _MIN_AREA;
        const T _MAX_AREA_FACTOR;
        const T _PADDING;
        const T _LAMBDA;
        const T _OUTPUT_SIGMA_FACTOR;
        const T _SCALE_STEP;
        const T _SCALE_WEIGHT;
        const T _INTERP_FACTOR;
        const T _KERNEL_SIGMA;
        const T _PSR_THRESHOLD;
        const int _TEMPLATE_SIZE;
        const int _PSR_PEAK_DEL;
        const int _CELL_SIZE;
        const int _N_SCALES_VOT;
        const int _PIXEL_PADDING;
        const int _RESIZE_TYPE;
        const bool _USE_VOT_SCALE_ESTIMATION;
        const bool _ENABLE_TRACKING_LOSS_DETECTION;
        const bool _USE_CCS;

        // it should be possible to find more reasonable values for min/max scale; application dependent
        const T _VOT_MIN_SCALE_FACTOR;
        const T _VOT_MAX_SCALE_FACTOR;

		T position[4];
    };
}

#endif
