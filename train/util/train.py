import os,sys
import shutil

path =  os.getcwd()
xml_path = '%s/%s'%(path,(sys.argv[1]))
print path,xml_path


os.system('opencv_traincascade -data %s -vec pos.vec -bg neg.txt -numPos %d -numNeg 8000 -numStages 18 --precalcValbufSize 1024 -precalcdxBufSize 1024 -featureType HARR -w 20 -h 20 -numThread 24'%(sys.argv[1],int(sys.argv[2])))
