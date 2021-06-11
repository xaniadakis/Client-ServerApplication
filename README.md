# Client-Server Application
The aim of this work was to get us acquainted with threads and web sockets.

![image](https://user-images.githubusercontent.com/75081526/121636773-1701e180-ca91-11eb-8a5a-779287f3a513.png)

As part of this task I had to change the distributed travelMonitor tool (https://github.com/xaniadakis/travelMonitor) I have created, so as to use threads, while communication between the parent process and the monitors is accomplished over sockets. The general idea and functionality of the work is similar
to the travelMonitor application, ie here too a central process accepts requests from citizens who
want to travel to other countries, checks if they have been properly vaccinated, and 
approves whether a traveler is allowed to enter a country. Specifically, I implemented a 
travelMonitorClient application which creates a series of monitor processes that, in cooperation with the
travelMonitorClient, answers user queries. In very general terms, in comparison to the travelMonitor application the differences are that, here the communication between the parent proccess (travelMonitorClient) and the monitor processes (monitorServers) is accomplished through web sockets and the proccessing of the files and records is accomplished with threads and a circular buffer. 

The application can be run as follows (inside the src subdirectory):

    make -s cleanall all run

The user can give the following commands to the application:

    /travelRequest citizenID date countryFrom countryTo virusName

The application first checks the bloom filter sent to it by the Monitor process that manages countryFrom. If the bloom filter indicates that the citizenID citizen has not been vaccinated against virusName prints a corresponding message. Elsewise the application requests via named pipe from the Monitor process that manages the countryFrom country if the citizenID has indeed been vaccinated. The application checks if the citizen has been vaccinated less than 6 months before the desired date travel date and prints a corresponding message.

    /travelStats virusName date1 date2 [country]

If no country argument is given, the application prints the number of citizens who have requested approval to travel in the period between [date1 ... date2] and the number of citizens approved and rejected. If given country argument, the application prints the same information but only for that country.

    /addVaccinationRecords country

With this request the user has placed in input_dir/country one or more files for
processing. The parent (travelMonitorClient) process sends a notification via socket to
monitorServer process that manages the country that there are input files to read in
directory. The monitorServer process reads whatever new file it finds, updates the data structures, and
sends back to the parent process, via socket, the updated bloom filters that represent the
group of vaccinated citizens. More specifically, the initial (listening) thread puts in the circular
buffer whatever new file it finds for reading and editing from the numThreads threads which will
update data structures. Once the processing of the new input files is finished, a thread
sends back to the parent process the updated bloom filters that represent the set of citizens that
have been vaccinated in the countries managed by the monitorServer process.

    /searchVaccinationStatus citizenID

The parent process forwards to all Monitor processes the request through named pipes. The Monitor process which manages the citizen with a citizenID ID sends through named pipe whatever information has for the vaccinations that the specific citizen has done/has not done. When the parent receives the information, he redirects them to stdout.

    /exit

Exit the application. The parent process sends an exit command to the monitorServers, via socket and prints to a file named log_file.xxx
where xxx is its process ID, the name of all the countries (subdirectories) that participated in the
data application, the total number of requests received to enter the countries, and the total
number of requests approved and rejected. Before it terminates, it obviously releases all of the allocated memory. 
The same procedure is followed by the monitorServers.

There also exists a bash script that creates the input_dir directory that is used as input for the main application. The script can be run as follows:

    ./create_infiles.sh inputFile input_dir numFilesPerDirectory

It uses as input the inputFile file created from another bash script "testFile.sh" which can be run as follows:

    ./testFile.sh virusesFile countriesFile numLines duplicatesAllowed 
