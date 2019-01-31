# RosBridge Demo for Madara and Gams

## Setup

To setup Madara and Gams execute the following code
```bash
git clone git@github.com:auerj/gams_rosbridge.git
cd gams_rosbridge
./build_gams.sh/
```
Build the bridge
```bash
./build.sh
```

# Simple Bridge Example
You will need four terminal windows.

## Terminal 1 (rosmaster):
```bash
rosmaster
```

## Terminal 2 (rostopic):
Publish a simple rostopic (Odometry) with 1 Hz by calling:
```bash
rostopic pub my_topic nav_msgs/Odometry "header:
  seq: 0
  stamp:
    secs: 0
    nsecs: 0
  frame_id: ''
child_frame_id: ''
pose:
  pose:
    position: {x: 1.1, y: 2.2, z: 3.3}
    orientation: {x: 0.0, y: 0.0, z: 0.0, w: 0.0}
  covariance: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
twist:
  twist:
    linear: {x: 1.0, y: 2.0, z: 3.0}
    angular: {x: 0.0, y: 0.0, z: 0.0}
  covariance: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0]" -r 1
```

## Terminal 3 (rosbridge):
Launch the rosbridge by calling:
```bash
source activate
./rosbridge
```
This will print the KnowledgeBase every second. This should look similar to that:
```
Knowledge in Knowledge Base:
.gams.frames.world.inf.origin=0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000
.gams.frames.world.inf.toi=11817896236130
.id=0
.platform.debug_printer.communication_available=0
.platform.debug_printer.deadlocked=0
.platform.debug_printer.failed=0
.platform.debug_printer.gps_spoofed=0
.platform.debug_printer.movement_available=0
.platform.debug_printer.moving=0
.platform.debug_printer.ok=1
.platform.debug_printer.reduced_movement=0
.platform.debug_printer.reduced_sensing=0
.platform.debug_printer.sensors_available=0
.platform.debug_printer.waiting=0
.prefix=agent.0
agent.0.algorithm.debug.deadlocked=0
agent.0.algorithm.debug.failed=0
agent.0.algorithm.debug.finished=0
agent.0.algorithm.debug.ok=1
agent.0.algorithm.debug.paused=0
agent.0.algorithm.debug.unknown=0
agent.0.algorithm.debug.waiting=0
sensors.odom.child_frame_id=
sensors.odom.frame_id=
sensors.odom.pose=1.100000, 2.200000, 3.300000, 0.000000, 0.000000, 0.000000
sensors.odom.pose.covariance=0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000
sensors.odom.twist.angular=0.000000, 0.000000, 0.000000
sensors.odom.twist.covariance=0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000
sensors.odom.twist.linear=1.000000, 2.000000, 3.000000
swarm.size=-1
```
In the namespace `sensors.odom` you can see the converted values from the rostopic.