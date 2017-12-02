from glob import glob
import os,sys
import random
path = os.getcwd()
print path

i = 0

f = open('%s/pos.txt'%path,'w')
filelist = glob('%s/pos/*.jpg'%path)
for name in filelist:
    f.write('%s 1 0 0 20 20\n'%name)
    i+=1

aug = 0
if aug:
    filelist = glob('%s/a_pos/*.png'%path)
    for name in filelist:
        if random.randint(0,5) == 2: 
            f.write('%s 1 0 0 100 100\n'%name)
            i+=1
num = i-1
os.system('opencv_createsamples -vec pos.vec -info pos.txt -w 20 -h 20 -num %d'%num)

