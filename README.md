# viskerobot
Bakalaureusetöö "Koostööroboti tööulatuse suurendamine tööobjekti viskamise teel" raames koostatud programmid kuubiku viskamiseks X-ARM 7 koostöörobotiga

Demo programs for throwing a cube with X-ARM 7 collaborative robot, created for the Bachelor's thesis "Increasing the working range of a collaborative robot by throwing the work object". 


## EST
### Eeldused
Riistvara: koostöörobot X-ARM 7, UFACTORY X-ARM kontroller ja 2 sõrmega haarats. Demos on visatava objektina kasutatud 32 mm küljepikkusega plastmassist kuubikut ning sihtmärgina papist topsi välisläbimõõduga 75 mm ning kõrgusega 80 mm. 

Tarkvara: 
1. Ubuntu 24.04 - [juhend](https://ubuntu.com/tutorials/install-ubuntu-desktop#1-overview)
2. ROS2 Jazzy - [juhend](https://docs.ros.org/en/jazzy/Installation.html)
3. xArm ROS2 package - [juhend](https://github.com/xArm-Developer/xarm_ros2/tree/jazzy#4-how-to-use)


### Demo kasutamine
Demode paketi lisamine alammoodulina:
```bash
git submodule add https://github.com/kkuiva/viskerobot
```

## ENG
### Requirements
Hardware: cobot X-ARM 7, UFACTORY X-ARM controller and 2 finger gripper. In the demo, the object being thrown is a plastic cube with the length of 32 mm and the goal is a cardboard cup with the outer diameter of 75 mm and height 80 mm. 

Software: 
1. Ubuntu 24.04 - [guide](https://ubuntu.com/tutorials/install-ubuntu-desktop#1-overview)
2. ROS2 Jazzy - [guide](https://docs.ros.org/en/jazzy/Installation.html)
3. xArm ROS2 package - [guide](https://github.com/xArm-Developer/xarm_ros2/tree/jazzy#4-how-to-use)


### Using the demo
Adding the package as a submodule:
```bash
git submodule add https://github.com/kkuiva/viskerobot
```
