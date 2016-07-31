% Tutorial
% Lujing Cen
% 7/30/2016

# Introduction

Are you trying to build a quadruped robot? Are you trying to understand code in this repository? If you answered yes to either question, you are in the right place. This tutorial will provide a comprehensive guide for building a quadruped robot while providing details on how important pieces of code in this repository function and relate to their conceptual counterparts.

### Warnings

This guide is in no way comprehensive. I am not accountable for errors in this document. Don't let this guide prevent you from thinking of new designs, concepts, and algorithms. Remember that I am no expert on this subject. Your ideas could very well be better than mine. At the same time, don't attempt to reinvent the wheel. If something is efficient and works for your purpose, use it.

"Premature optimization is the root of all evil." Test and measure. Don't guess.

### Read Me

While researching, I chanced upon the *Springer Handbook of Robotics*. Before proceeding, read section 16.5 of the book found [here](http://home.deib.polimi.it/gini/robot/docs/legged.pdf). If you are serious about robotics, I recommend that you find this book in the library or get an eBook. This book basically covers everything you need to know about advanced robotics and even provides historical background. You probably won't understand everything in this literature. That is completely okay. This is just an introduction.

### Prerequisites

Trying to accomplish this project requires that you are not scared of math and physics. You will need a strong foundation in geometry, trigonometry, algebra, classical mechanics, and prelimiary linear algebra. You will also need a deep understanding of calculus, although you might not ever need to take a derivative in this project. It would help that you also understand some practical electricity and magnetism.

# Definitions

It is important to define everything before starting this project. It will save you a lot of time. Don't invent your own coordinate system like me and waste 2 weeks :p.

### Coordinate System

The world runs on the right-handed coordinate system. Being left-handed, I accidentally used the left-handed coordinate system, which ended up causing me a lot of confusion. Don't be like me. Clearly define the coordinate system before beginning. This is not to say you can't use the left-handed system. Just make sure you know what you're doing, because pretty much everything on the internet uses the right-handed system.

![](assets/table.jpg)

Pretend the robot is a table. The head points in the positive x direction. The left side points in the positive y direction. The top of the table points to the positive z axis. The image is there for clarification.

### Vocabulary

End Effector
: In kinematics, the end of a robotic arm or leg. In this case, it is the tip of the feet.

Root
: In kinematics, the point at which the arm or leg attaches to the body.
	
# Leg Design

### Alignment

### Considerations

# Kinematics

This is perhaps the most math intensive and important portion of the project.

### Forward Kinematics

### Inverse Kinematics

### Finesse (class)

# Servos

### Configuration

### Information

### Servo (class)

# Mini Maestro

### Protocol

### USC

### Synchronization

### Maestro (class)

# Movement

We want to translate user inputs of rotational motion ($rad / sec$) and forward motion ($cm / sec$) to physical robot motion. The first challenge is to use $d\theta$, the amount of rotation per cycle, and $dv$, the amount of forward motion per cycle, to generate proper gaits. The second challenge is to perform various gait optimizations to ensure that the robot walks smoothly.

### Vectors

My friend Alastair MacMillan came up with this amazing idea one day that basically made complex motion possible. For a moment, pretend that the robot is just a rectangle.

<div class="center">
![](assets/vector.png)\ ![](assets/vector1.png)
</div>

The verticies or legs are number 0-3 accordingly. To rotate the rectangle, we can simply apply a vector at each vertex. The cool thing about vectors is that they can be view as separate components. To add forward or backward movement, simply add to the x component.

The easiest method to physically reproduce the vector is to compute $(x, y)$ and generate a line between $(x, y)$ and $(-x, -y)$. Moving one cycle along the line will be twice the desired vector. 

The harder part is using $\theta$ to find $x$ and $y$. At every instantaneous moment in time, each leg applies a force. Relative to their respective roots the force is linear over time. However, when all four legs work together, the linear force becomes rotational. 

![](assets/vector2.png)

We define the width $w$ to be the distance along y and length $l$ to be the distance along x. Let $u$ be a matrix or width and length. The magnitude of the vector is defined to be $|\Delta|$ and the radius of the circle is $r$. A few things become immediately evident.

$$r = \frac{1}{2} \sqrt{w^2 + l^2}$$
$$|\Delta| = \sqrt{x^2 + y^2}$$
$$u = \begin{bmatrix} w \\ l \end{bmatrix}$$

$\theta$, how much the robot will turn when the leg moves $\Delta$, is between $r$ and the hypotenuse. It can be solved using inverse tangent.

$$\theta = \arctan \left(\frac{|\Delta|}{r}\right)$$

The desired theta input is twice of that. So the input $d\theta = 2 \times \theta$. We can then solve for $|\Delta|$.

$$|\Delta| = r \times \tan \left(\frac{1}{2} d\theta \right)$$

The $(x, y)$ contribution from the rotation part is always perpendicular to the line from the center to the vertex. This means that we can use the [normalized vector](https://en.wikipedia.org/wiki/Unit_vector) $\hat{u}$ to find the desired values. Finally, add the desired rotational contribution value for $dv$.

$$\begin{bmatrix} x \\ y \end{bmatrix} = |\Delta| \times \hat{u} + \begin{bmatrix}dv \\ 0 \end{bmatrix}$$

### Gait Types

There are various gait types described in *Springer Handbook of Robotics*. However, I believe that trot and crawl are the two easier gaits to implement. Obviously, trot is much faster than crawl. However, testing should reveal the optimal transition speed and time. If both are graphed, if can be seen that this is an optimization problem. I did not have sufficient time to test thoroughly. However, if time $t$ can be found, it is easy to convert $rad / sec$ and $cm / sec$ to $d\theta$ and $dv$.

$$\begin{bmatrix} d\theta \\ dv \end{bmatrix} = t \times \begin{bmatrix} rad/sec \\ cm/sec \end{bmatrix}$$

I have not found significant differences between different leg orderings. However, this is because I have not done extensive testing. The robots use 1423 creeping gaits, although the other orderings work as well.

### Gait Generation

### Gait Execution

### Center of Mass

### Static Optimization

### Dynamic Optimization

### Dynamic (class)

### Agility (class)

# Vision

### Tracking

### Detection

### SLAM

### Optimization

### Oculus (class)

### Theia (class)

# Audio

### Concepts

### Lykos

# Automation

### Synthesis

### Decision Making

### Ares (class)

# Communication

### Workers

### Networking

### Cerebral (class)

# Control

### Considerations

### Phi (class)

# Material Selection

### Body

### Legs

# Component Selection

### Servos

### Steppers

### Microcontroller

### Battery

### Camera

# Electrical

### Amperage Draw

### Safety

### Battery
