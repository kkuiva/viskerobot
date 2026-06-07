# viskerobot
Bakalaureusetöö "Koostööroboti tööulatuse suurendamine tööobjekti viskamise teel" raames koostatud programmid kuubiku viskamiseks X-ARM 7 koostöörobotiga. 
Koostati kolm demoprogrammi:
1. Vise maksimumkiirusel, rakendades 4. liigendit;
2. Vise maksimumkiirusel, rakendades 4. ja 6. liigendit;
3. Vise kauguse põhjal arvutatud kiirusel, rakendades 4. liigendit.

Demo programs for throwing a cube with X-ARM 7 collaborative robot, created for the Bachelor's thesis "Increasing the working range of a collaborative robot by throwing the work object". 
3 demo programs were created:
1. Throwing at maximum speed, using the 4th joint;
2. Throwing at maximum speed, using the 4th and 6th joints;
3. Throwing at speed calculated from the desired distance, using the 4th joint.



## EST
### Eeldused
Riistvara: koostöörobot X-ARM 7, UFACTORY X-ARM kontroller ja 2 sõrmega haarats. Demos on visatava objektina kasutatud 32 mm küljepikkusega plastmassist kuubikut ning sihtmärgina papist topsi välisläbimõõduga 75 mm ning kõrgusega 80 mm. 

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
Demo koostamisel kasutatud robot oli kinnitatud 19 cm kõrgusele alusele. Enne demo kasutamist on oluline veenduda selles, et robotil on võimalik liikuda ohutult igasse kasutatud punkti, mis on leitavad programmidest throwing_pub, throwing_pub2 ja throwing_pub_math. Selleks on võimalik kasutada xarm_ros2 paketi roboti käsurealt juhtimise võimalust, mida on kirjeldatud [siin](https://github.com/xArm-Developer/xarm_ros2/tree/jazzy/xarm_api#1-starting-xarm-by-ros-service).
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
Seejärel on robot liikumiskäskude täitmiseks valmis. Roboti kindlasse positsiooni liigutamiseks saab kasutada järgmist käsku, kus defineeritakse positsioon ühikutes mm ja rad, kiirus ja kiirendus ühikutes mm/s ja mm/s2 ning liikumise ajapiirang sekundites. Oluline on meeles pidada, et roboti tööriistakoordinaadistik on baaskoordinaadistiku suhtes 3,14 rad pööratud, seega peab roboti "normaalse" asendi puhul pöördenurk olema 3,14:
```bash
ros2 service call /xarm/set_position xarm_msgs/srv/MoveCartesian "{pose: [300, 0, 250, 3.14, 0, 0], speed: 50, acc: 500, mvtime: 0}" 
```
Roboti kindlasse poosi liigutamiseks saab kasutada järgmist käsku, kus defineeritakse poos ühikutes rad, kiirus ja kiirendus ühikutes rad/s ja rad/s2 ning liikumise ajapiirang sekundites:
```bash
ros2 service call /xarm/set_servo_angle xarm_msgs/srv/MoveJoint "{angles: [-0.58, 0, 0, 0, 0, 0, 0], speed: 0.35, acc: 10, mvtime: 0}"
```
Teistsuguste mõõtmetega kuubiku kasutamisel on oluline parandada ka haaratsi avatud ja suletud asendeid, mis on leitavad programmide throwing_pub, throwing_pub2 ja throwing_pub_math demotsüklitest. Haaratsi asend defineeritakse ühikutes rad, erinevate asendite katsetamiseks saab kasutada järgmist käsku, mida on kirjeldatud [siin](https://github.com/xArm-Developer/xarm_ros2/tree/humble/xarm_api#8-gripper-control). Haaratsi positsioon antakse vahemikus 0 (avatud) kuni 0,86 (suletud), max_effort võimaldab määrata suurima rakendatava jõu, kuid haaratsi G1 puhul ei ole võimalik seda haaratsile edastada:
```bash
ros2 action send_goal /xarm_gripper/gripper_action control_msgs/action/GripperCommand "{command: {position: 0.5, max_effort: 0}}"
```

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
ros2 run viskerobot throw_pub

# Vise maksimumkiirusel, rakendades 4. ja 6. liigendit:
ros2 run viskerobot throw_pub2

# Vise kauguse põhjal arvutatud kiirusel, rakendades 4. liigendit:
ros2 run viskerobot throw_pub_math
```


## ENG
### Requirements
Hardware: cobot X-ARM 7, UFACTORY X-ARM controller and 2 finger gripper. In the demo, the object being thrown is a plastic cube with the length of 32 mm and the goal is a cardboard cup with the outer diameter of 75 mm and height 80 mm. 

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
