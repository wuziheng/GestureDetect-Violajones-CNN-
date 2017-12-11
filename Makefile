TOP_PATH=$(shell pwd)
SRC=$(TOP_PATH)/src
INCLUDE=$(TOP_PATH)/include
LIB=$(TOP_PATH)/lib

CAFFE_ROOT=/home/firefly/CaffeOnACL
CAFFE_INCS += -I$(CAFFE_ROOT)/include
CAFFE_INCS += -I$(CAFFE_ROOT)/build/src


CXX=g++
CFLAGS=-std=c++11 -Wall -g -O3 
CFLAGS+= $(CAFFE_INCS) -DCPU_ONLY=1 -Wno-sign-compare
CFLAGS+= -I/usr/local/include  
CFLAGS+= -I/usr/local/AID/opencv3.3.0/include  
CFLAGS+= -I$(INCLUDE)

# locate ldconfig -p|grep "libxxx"
# define the util ldflags
ULDFLAGS+=-L/usr/local/AID/opencv3.3.0/lib -lopencv_imgproc -lopencv_core -lopencv_highgui 

# define the cnn ldflags
CLDFLAGS+=-L/usr/local/AID/opencv3.3.0/lib -lopencv_highgui
CLDFLAGS+=-Wl,-rpath,$(CAFFE_ROOT)/build/lib -L$(CAFFE_ROOT)/build/lib -lcaffe
CLDFLAGS+=-lprotobuf -lglog

# define the main ldflags
LDFLAGS+=-L/usr/local/AID/opencv3.3.0/lib -lopencv_highgui -lopencv_videoio -lopencv_video -lopencv_imgproc -lopencv_core -lopencv_imgcodecs -lopencv_objdetect
LDFLAGS+=-lboost_system
LDFLAGS+=-L/usr/local/lib -lgflags -lpthread
LDFLAGS+=$(LIB)/libutil.so $(LIB)/libcnn.so


test: test.cpp util cnn 
	$(CXX) -o test test.cpp  ${CFLAGS} ${LDFLAGS} 


util: $(LIB)/libutil.so
$(LIB)/libutil.so: $(SRC)/util.cpp $(INCLUDE)/util.h
	$(CXX) $(SRC)/util.cpp ${CFLAGS} ${ULDFLAGS}  -lopencv_core -fpic -shared -o $(LIB)/libutil.so 


cnn: $(LIB)/libcnn.so
$(LIB)/libcnn.so: $(SRC)/handcnn.cpp $(INCLUDE)/handcnn.hpp
	$(CXX) $(SRC)/handcnn.cpp $(CFLAGS) $(CLDFLAGS) -fpic -shared -o $(LIB)/libcnn.so

clean: 
	rm -f $(LIB)/*.so $(LIB)/*.a $(LIB)/*o
	rm -f *.d *.o *.so
	rm -f test

