Problem
===
In a building, there are eight meeting rooms, which are shared by 10 departments on an as-needed basis. Each department has a secretary who is responsible for resvering meeting rooms. Each resvervation slot is of 1 h duration, and rooms can only be resverved between 8 a.m. and 5 p.m. Your job is to implement a shared room revervation schedule that can be used by the secretary.<br>
Use the following conflict resolution rule: If multiple appointments overlap, the earliest entry for that slot revails and others are cancelled. If there is a tie for the earliest entry, the secretaries' first names will be used to break the tie. Each secretary will work on a separeate replica of the schedule and enter the reservation request stamped by the clock time. The clocks are synchronized. Dependency checks will be done during the antientropy sessions as in Bayou. The goal is eventual consistency.<br>
solution
===
In the given program, a distributed system  is designed for reserving eight meeting room shared by 10 Departments on an as-needed basis. The most challenge is how to deal with concurrency event in distributed system, like two or more people order one same room at the same time. At this program, I used Bayou solution to solve concurrency in distributed system.

Here is the whole Architecture for this system.<br>
![](http://debuggingnow.com/my-blog/wp-content/uploads/2015/08/bayou2.jpg)
<br> Figure: Interaction between terminals, replicas and primary server<br>
The sequence of message exchange for the reservation of meeting room can be explained as follow:

* 1,Write: The department Secretary wanting to reserve room send a write request to the replica<br>
* 2,Request: The replica receiving write request sends a time stamped Request message to the primary server.<br>
* 3,Reply: The primary server respond to the request message with a reply message<br>
* 4,Update: After the replica receive the reply from primary server; it updates its data indicating that the room is reserved for the requested day and time.<br>
* 5,Release: The replica after updating itself sends a release message to the primary server.<br>
* 6,Release: The primary server then broadcast release message to all of the replica to update the status of the reservation to the local database.<br>

Sequence Diagram
=====================
<br>The bellowing sequence diagram shows  how system works for reserving room that effectively circumscribe all the possible cases that might occur in process of reservation:<br>
![](http://debuggingnow.com/my-blog/wp-content/uploads/2015/08/bayou.png)

