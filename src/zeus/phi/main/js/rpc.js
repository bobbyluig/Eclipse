ctrlRpc.basicDecision = function (args, kwargs, details) {
    var d = new autobahn.when.defer();

    var functions = {};
    var strings = {
        header: args[0],
        content: args[1]
    };
    var context = {run: functions, data: strings};

    functions.yes = function () {
        d.resolve(true);
        ctrlLog.log('system', 'A user decision has been resolved.', 0);
    };
    functions.no = function () {
        d.resolve(false);
        ctrlLog.log('system', 'A user decision has been resolved.', 0);
    };

    ctrlLog.log('system', 'A user decision has been requested.', 0);
    ctrlModal.modal('choice', true, context);

    return d.promise;
};

ctrlRpc.registerAll = function() {
    wamp.register('zeus.phi.basic_decision', ctrlRpc.basicDecision);
    ctrlLog.log('system', 'Registered all procedures.', 1);
};