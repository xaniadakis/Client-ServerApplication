# OBJS 		= $(filter-out Monitor.o, $(wildcard *.o))
# SOURCE		= $(wildcard *.c)
PARENTOBJS 	= travelMonitorClient.o travelMonitorClientUtil.o myLib.o hashTable.o bloomFilter.o skipList.o hashFunctions.o 
CHILDOBJS	= monitorServer.o monitorServerUtil.o myLib.o hashTable.o bloomFilter.o skipList.o hashFunctions.o circularBuffer.o
SOURCE		= travelMonitorClient.c travelMonitorClientUtil.c monitorServer.c monitorServerUtil.c myLib.c hashTable.c bloomFilter.c skipList.c hashFunctions.c circularBuffer.c monitorSignals.c parentSignals.c
HEADER		= $(wildcard ../lib/*.h)
OUT1  		= travelMonitorClient
OUT2  		= monitorServer
CC			= gcc
FLAGS  		= -c #-O3
DEBUGFLAGS	= -g -c -Wall -Wextra # -pedantic -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror #-Wsign-conversion

debug 				?= 0
virusesfile 		?= smallvirusesFile
countriesfile	 	?= smallcountriesFile
numlines 			?= 200
duplicatesallowed 	?= 1
numMonitors			?= 3
numThreads			?= 2
sizeOfBloom			?= 10000
cyclicBufferSize	?= 2
socketBufferSize	?= 1000
input_dir			?= ../script/input_dir
inputfile 			?= inputFile
numFilesPerDirectory?= 3
scr_input_dir 		?= input_dir

all:	script	\
	$(OUT2)	\
	$(OUT1)	
	
run:
	./$(OUT1) -m $(numMonitors) -b $(socketBufferSize) -c $(cyclicBufferSize) -s $(sizeOfBloom) -i $(input_dir) -t $(numThreads)

valgrind:	
#	valgrind --trace-children=yes --track-fds=yes valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file="valgrind-out.txt" ./$(OUT) -m $(nummonitors) -b $(buffersize) -s $(bloomsize) -i $(input_dir)
#	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file="valgrind-out.txt"  ./travelMonitor -m 3 -b 1000 -s 1000 -i ../script/input_dir
#	--trace-children=yes
#	valgrind --show-leak-kinds=all --trace-children=yes --track-origins=yes --leak-check=full --log-file="valgrind-out1.txt" -v ./$(OUT) -m 1 -b $(buffersize) -s $(bloomsize) -i $(input_dir)
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --track-origins=yes ./$(OUT1) -m $(numMonitors) -b $(socketBufferSize) -c $(cyclicBufferSize) -s $(sizeOfBloom) -i $(input_dir) -t $(numThreads)

$(OUT1): $(PARENTOBJS) 
	$(CC) -g -D_XOPEN_SOURCE=700 -o $@ parentSignals.c $(PARENTOBJS) -lm


$(OUT2): $(CHILDOBJS) 
	$(CC) -g -D_XOPEN_SOURCE=700 -o $@ monitorSignals.c $(CHILDOBJS) -lm -pthread

travelMonitorClient.o: travelMonitorClient.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) travelMonitorClient.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) travelMonitorClient.c 
else
	$(CC) $(FLAGS) travelMonitorClient.c  
endif
endif

monitorServer.o: monitorServer.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) monitorServer.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) monitorServer.c 
else
	$(CC) $(FLAGS) monitorServer.c  
endif
endif

travelMonitorClientUtil.o: travelMonitorClientUtil.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) travelMonitorClientUtil.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) travelMonitorClientUtil.c 
else
	$(CC) $(FLAGS) travelMonitorClientUtil.c  
endif
endif

monitorServerUtil.o: monitorServerUtil.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) monitorServerUtil.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) monitorServerUtil.c 
else
	$(CC) $(FLAGS) monitorServerUtil.c  
endif
endif

myLib.o: myLib.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) myLib.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) myLib.c
else
	$(CC) $(FLAGS) myLib.c
endif
endif

hashTable.o: hashTable.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) hashTable.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) hashTable.c
else
	$(CC) $(FLAGS) hashTable.c
endif
endif

bloomFilter.o: bloomFilter.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) bloomFilter.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) bloomFilter.c
else
	$(CC) $(FLAGS) bloomFilter.c
endif
endif

skipList.o: skipList.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) skipList.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) skipList.c
else
	$(CC) $(FLAGS) skipList.c
endif
endif

hashFunctions.o: hashFunctions.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) hashFunctions.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) hashFunctions.c
else
	$(CC) $(FLAGS) hashFunctions.c
endif
endif

circularBuffer.o: circularBuffer.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) circularBuffer.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) circularBuffer.c
else
	$(CC) $(FLAGS) circularBuffer.c
endif
endif

.PHONY:	script
script:	../script/create_infiles.sh
	cd ../script; ./testFile.sh $(virusesfile) $(countriesfile) $(numlines) $(duplicatesallowed);
	cd ../script; rm -rf input_dir; ./create_infiles.sh $(inputfile) $(scr_input_dir) $(numFilesPerDirectory); 

.PHONY:	clean
clean:
	rm -f $(CHILDOBJS) $(PARENTOBJS) $(OUT1) $(OUT2)

.PHONY:	cleanall
cleanall:
	rm -f $(CHILDOBJS) $(PARENTOBJS) $(OUT1) $(OUT2) 
	rm -f *.o
	rm -f ../script/inputFile
	rm -rf ../script/input_dir
	rm -f valgrind-out.txt
	rm -f ../tmp/namedPipes/*
	rm -rf ../log_folder
	rm -rf ../tmp