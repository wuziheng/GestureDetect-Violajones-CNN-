import os,sys
import shutil

path =  os.getcwd()
xml_path = '%s/%s'%(path,(sys.argv[1]))
print path,xml_path

for i in range(5,19):
    os.system('opencv_traincascade -data %s -vec pos.vec -bg neg.txt -numPos 250 -numNeg 3000 -numStages %d --precalcValbufSize 1024 -precalcdxBufSize 1024 -featureType HARR -w 20 -h 20 -numThread 24'%(sys.argv[1],i))
    shutil.move('%s/cascade.xml'%xml_path,'%s/cascade_%d.xml'%(xml_path,i))
