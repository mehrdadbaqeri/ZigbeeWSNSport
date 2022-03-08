# ZigbeeWSNSport
Make use of Wireless Sensor Network and Zigbee for monitoring athletes body parameters

Introduction
In most sport application, online monitoring behavior sounds to be interesting so that the coach can analyses his/her players even during the match and makes better decision for tired players, hopefully prevents any horrible events like sudden heart attack. So we develop a wireless networks to monitor athlete behavior in the sense of temperature and number of steps taken by player. The sensors in this application capture raw data and then capsulate it in the specific packet format. Then at the specified time or after any changes they send information to based station through mesh network.
At based station (DB) side, the received packet is decapsulated and saved on the DB for further analyses. Obviously due to the memory limitation at mote, laptop instead can be a good interface and storage platform. Rather than temperature or step counts data, other network statics information like packet lost, data propagation path, etc are propagated by a packet which are interesting for monitoring and debugging of network  status.

![image](https://user-images.githubusercontent.com/7360143/157331146-c10b35d6-2563-488a-ad62-e8d7b90c76a3.png)

# Hardware platform
[Zolteria Z1](http://wiki.zolertia.com/wiki/index.php/Main_Page)

# Customized packet to send:
![image](https://user-images.githubusercontent.com/7360143/157330922-f1d35d14-ac1f-40eb-a26c-8cf085daac09.png)
