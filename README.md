# viskerobot
Bakalaureusetöö "Koostööroboti tööulatuse suurendamine tööobjekti viskamise teel" raames koostatud programmid kuubiku viskamiseks X-ARM 7 koostöörobotiga. 
Koostati kolm demoprogrammi:
1. Vise maksimumkiirusel, rakendades 4. liigendit;
2. Vise maksimumkiirusel, rakendades 4. ja 6. liigendit;
3. Vise kauguse põhjal arvutatud kiirusel, rakendades 4. liigendit.

4. liigendiga visates saavutati viskekaugus 1090 mm ning 4. ja 6. liigendiga 1280 mm roboti baasist.


Demo programs for throwing a cube with X-ARM 7 collaborative robot, created for the Bachelor's thesis "Increasing the working range of a collaborative robot by throwing the work object". 
3 demo programs were created:
1. Throwing at maximum speed, using the 4th joint;
2. Throwing at maximum speed, using the 4th and 6th joints;
3. Throwing at speed calculated from the desired distance, using the 4th joint.

The maximum throwing distance achieved, measured from the robot base, was 1090 mm when using the 4th joint and 1280 mm when using the 4th and 6th joint. 


## EST
### Eeldused
Riistvara: koostöörobot X-ARM 7 (kinnitatud 19 cm kõrgusele alusele), UFACTORY X-ARM kontroller ja 2 sõrmega haarats. Demos on visatava objektina kasutatud 32 mm küljepikkusega plastmassist kuubikut ning sihtmärgina papist topsi välisläbimõõduga 75 mm ning kõrgusega 80 mm (paigutatud lauale, millel oli roboti alus). 

Tarkvara: 
1. Ubuntu 24.04 - [juhend](https://ubuntu.com/tutorials/install-ubuntu-desktop#1-overview)
2. ROS2 Jazzy - [juhend](https://docs.ros.org/en/jazzy/Installation.html)
3. xArm ROS2 package - [juhend](https://github.com/xArm-Developer/xarm_ros2/tree/jazzy#4-how-to-use)


### Demo kasutamine
Demode paketi tööruumi lisamine alammoodulina:
```bash
cd ~/tööruumi_nimi/src/
git submodule add https://github.com/kkuiva/viskerobot
```
Tööruumi ehitamine:
```bash
cd ~/tööruumi_nimi/
colcon build
```

Enne demo kasutamist on oluline veenduda selles, et robotil on võimalik liikuda ohutult igasse kasutatud punkti, mis on leitavad programmidest throwing_pub, throwing_pub2 ja throwing_pub_math. Selleks on võimalik kasutada xarm_ros2 paketi roboti käsurealt juhtimise võimalust, mida on kirjeldatud [siin](https://github.com/xArm-Developer/xarm_ros2/tree/jazzy/xarm_api#1-starting-xarm-by-ros-service).
Esmalt tuleb luua ühendus reaalse robotiga, kus roboti ip aadress sõltub kontrolleri ip aadressist:
```bash
cd ~/tööruumi_nimi/
ros2 launch xarm_api xarm7_driver.launch.py robot_ip:=192.168.1.117
```
Teises terminali aknas tuleb lubada roboti liigendite liikumine ning määrata selle töörežiim ja seisund:
```bash
cd ~/tööruumi_nimi/
ros2 service call /xarm/motion_enable xarm_msgs/srv/SetInt16ById "{id: 8, data: 1}"
ros2 service call /xarm/set_mode xarm_msgs/srv/SetInt16 "{data: 0}"
ros2 service call /xarm/set_state xarm_msgs/srv/SetInt16 "{data: 0}"
```
Seejärel on robot liikumiskäskude täitmiseks valmis. Roboti kindlasse positsiooni liigutamiseks saab kasutada järgmist käsku, kus defineeritakse positsioon ühikutes mm ja rad, kiirus ja kiirendus ühikutes mm/s ja mm/s^2. Oluline on meeles pidada, et roboti tööriistakoordinaadistik on baaskoordinaadistiku suhtes 3,14 rad pööratud, seega peab roboti "normaalse" asendi puhul pöördenurk olema 3,14:
```bash
ros2 service call /xarm/set_position xarm_msgs/srv/MoveCartesian "{pose: [300, 0, 250, 3.14, 0, 0], speed: 50, acc: 500}" 
```
Roboti kindlasse poosi liigutamiseks saab kasutada järgmist käsku, kus defineeritakse poos ühikutes rad, kiirus ja kiirendus ühikutes rad/s ja rad/s^2:
```bash
ros2 service call /xarm/set_servo_angle xarm_msgs/srv/MoveJoint "{angles: [-0.58, 0, 0, 0, 0, 0, 0], speed: 0.35, acc: 10}"
```

Teistsuguste mõõtmetega kuubiku kasutamisel on oluline parandada ka haaratsi avatud ja suletud asendeid. Demos kasutatakse avatud asendi jaoks väärtust 0,2 ning suletud asendi jaoks väärtust . Haaratsi asend defineeritakse ühikus rad, vahemikus 0 (avatud) kuni 0,86 (suletud). Erinevate asendite katsetamiseks saab kasutada järgmist käsku, mida on kirjeldatud [siin](https://github.com/xArm-Developer/xarm_ros2/tree/humble/xarm_api#8-gripper-control), kus max_effort võimaldab määrata suurima rakendatava jõu, kuid haaratsi G1 puhul ei ole võimalik seda haaratsile edastada:
```bash
ros2 action send_goal /xarm_gripper/gripper_action control_msgs/action/GripperCommand "{command: {position: 0.5, max_effort: 0}}"
```

Enne kauguse põhjal kiirust arvutava demo kasutamist on oluline leida viskepunkti ja sihtpunkti vaheline kõrgus, et kiirus arvutataks korrektselt. Selle kõrguse leidmiseks on oluline teada, et viskepunkt on roboti baasist umbes 26 cm kõrgemal ning juhul, kui sihtpunkt on anum, tuleks sihtpunktiks võtta anuma ülemise külje keskpunkt. Kui leitud kõrgus erineb demos kasutatud 35 cm-st, siis peab muutuja y väärtuse muutma programmis throwing_pub_math real 206. Juhul, kui sihtpunkt on viskepunktist madalamal, peab kõrgus olema negatiivne.


Pärast demos kasutatavate positsioonide, pooside ja haaratsi asendite kontrollimist saab demo käivitada. Selleks on vaja kasutada kolme terminali akent. 
Esimeses tuleb luua ühendus robotiga:
```bash
cd ~/tööruumi_nimi/
ros2 launch xarm_api xarm7_driver.launch.py robot_ip:=192.168.1.117
```
Teises tuleb käivitada haaratsi juhtimisega tegelev programm:
```bash
cd ~/tööruumi_nimi/
ros2 run viskerobot grip_w_subscriber
```
Kolmandas saab käivitada viskamise põhiprogrammi:
```bash
cd ~/tööruumi_nimi/

# Vise maksimumkiirusel, rakendades 4. liigendit:
ros2 run viskerobot throwing_pub

# Vise maksimumkiirusel, rakendades 4. ja 6. liigendit:
ros2 run viskerobot throwing_pub2

# Vise kauguse põhjal arvutatud kiirusel, rakendades 4. liigendit:
ros2 run viskerobot throwing_math
```


## ENG
### Requirements
Hardware: cobot X-ARM 7 (attached on a 19 cm tall support), UFACTORY X-ARM controller and 2 finger gripper. In the demo, the object being thrown is a plastic cube with the length of 32 mm and the goal is a cardboard cup with the outer diameter of 75 mm and height 80 mm, which was placed on the same desk with the robot's support. 

Software: 
1. Ubuntu 24.04 - [guide](https://ubuntu.com/tutorials/install-ubuntu-desktop#1-overview)
2. ROS2 Jazzy - [guide](https://docs.ros.org/en/jazzy/Installation.html)
3. xArm ROS2 package - [guide](https://github.com/xArm-Developer/xarm_ros2/tree/jazzy#4-how-to-use)


### Using the demo
Adding the demo package into your workspace as a submodule:
```bash
cd ~/workspace_name/src/
git submodule add https://github.com/kkuiva/viskerobot
```
building the workspace:
```bash
cd ~/workspace_name/
colcon build
```
The robot used in creating this demo was attached to a 19 cm tall support. Before using the demo, it's important to ensure that the robot can safely move to all of the points used, which can be found from files throwing_pub, throwing_pub2 and throwing_pub_math. This can be done by moving the robot into the positions using terminal commands from xarm_ros2 package, described [here](https://github.com/xArm-Developer/xarm_ros2/tree/jazzy/xarm_api#1-starting-xarm-by-ros-service).
First, you must connect to the robot, using the IP address that can be found on the controller:
```bash
cd ~/workspace_name/
ros2 launch xarm_api xarm7_driver.launch.py robot_ip:=192.168.1.117
```
In another terminal, you must enable the robot's joints to move and set the mode and state as "0":
```bash
cd ~/workspace_name/
ros2 service call /xarm/motion_enable xarm_msgs/srv/SetInt16ById "{id: 8, data: 1}"
ros2 service call /xarm/set_mode xarm_msgs/srv/SetInt16 "{data: 0}"
ros2 service call /xarm/set_state xarm_msgs/srv/SetInt16 "{data: 0}"
```
After that, the robot is ready for motion commands. Moving the robot to a specific position can be achieved with the following command, where the position is given in mm and rad, the speed and acceleration in mm/s and mm/s^2. It's important to remember that the tool coordinate system is rotated 3.14 rad around x-axis in relation to the base coordinate system, so for a "normal" robot position, the roll angle needs to be 3.14:
```bash
ros2 service call /xarm/set_position xarm_msgs/srv/MoveCartesian "{pose: [300, 0, 250, 3.14, 0, 0], speed: 50, acc: 500}" 
```
Moving the robot to a specific pose can be achieved with the following command, where the pose is defined in rad, speed and acceleration in rad/s and rad/s^2:
```bash
ros2 service call /xarm/set_servo_angle xarm_msgs/srv/MoveJoint "{angles: [-0.58, 0, 0, 0, 0, 0, 0], speed: 0.35, acc: 10}"
```
If you intend to use a cube with different measurement, the gripper open and close poses also need to be adjusted. The current values are 0.2 for open and  for closed. The gripper pose is defined in rad, which is given in the range of 0 (open) to 0.86 (closed). The gripper can be moved from command line with the following command, which is described [here](https://github.com/xArm-Developer/xarm_ros2/tree/humble/xarm_api#8-gripper-control), where max_effort is only relevant for gripper G2:
```bash
ros2 action send_goal /xarm_gripper/gripper_action control_msgs/action/GripperCommand "{command: {position: 0.5, max_effort: 0}}"
```

Before using the demo that calculates the speed based on desired distance, it is also important to verify that the height between the throwing position and the goal position is 35 cm, as used in the demo. For measuring and calculating this, consider that the throwing position is about 26 cm higher than the robot's base, and if you are using a container as the goal, the height should be considered from the top of the container. If this height is different, you must change the y value accordingly in the throwing_pub_math file, on line 206. If the goal point is below the throwing point, the value must be negative.

After testing the positions, poses and gripper poses used, the demo can be started. For this, three terminal windows are needed:
The first is used to connect to the robot:
```bash
cd ~/workspace_name/
ros2 launch xarm_api xarm7_driver.launch.py robot_ip:=192.168.1.117
```
The second is used to control the gripper:
```bash
cd ~/workspace_name/
ros2 run viskerobot grip_w_subscriber
```
The third is used to start the main throwing program:
```bash
cd ~/workspace_name/

# Throwing at max speed, using 4th joint:
ros2 run viskerobot throwing_pub

# Throwing at max speed, using 4th and 6th joint:
ros2 run viskerobot throwing_pub2

# Throwing at the speed calculated based on distance, using the 4th joint:
ros2 run viskerobot throwing_math
```
