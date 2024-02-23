# NS-3 Experiments
The propose of this experiment is to study point-to-point connection experiments.
The following describes the settings for this experiment.

Set up multiple client nodes, each node connected to the server through end-to-end. The number of clients and the number of packets sent can be set in the command line. The default number is 1.
The following describes the settings for this experiment. All client requests should be sent to the same IP address, which is 10.1.1.1.
Set a different random start time for the UdpEchoClient application on the client, between 2 and 7. For the UdpEchoClient application, the UdpEchoServer application, and the simulation itself, set the stop time to 20 seconds.
Use the SetAttribute() method to change the port number of the UdpEchoServer application to 15.
All other parameters should be the same as the values ​​in first.cc in ns-3, namely the data rate and latency for each end-to-end link, and the interval and packet size for each UdpEchoClient.
