from glob import glob
import cv2
import os
import random 

#if not os.path.exists('neg'):
#    os.mkdir('neg')
#
#neg_num = 2000
#
#negfile = open('neg.txt','w')
#filelist = glob('../neg/*.jpg')
#i = 1 
#for f in filelist:
#    if i == neg_num:
#        break
#
#    img  = cv2.imread(f)
#    try:
#        k = random.randint(1,5)
#        for j in range(k):
#            w = random.randint(0,img.shape[0]-21)
#            h = random.randint(0,img.shape[1]-21)
#            tmp = img[w:w+20,h:h+20,:]
#            cv2.imwrite('neg/%4d_%d.jpg'%(i,j),tmp)
#            negfile.write('neg/%4d_%d.jpg\n'%(i,j))
#            print i,j
#            
#            if i ==2000:
#                break
#            i+=1
#    except:
#        pass
VOCPATH = '/home/hard_ln/VOCdevkit/VOC2007/JPEGImages'
NEGPATH = '../neg'
FACEPATH= '/home/hard_ln/umdfaces_batch1/face'
CARPATH = '/home/hard_ln/car/cars_test/cars_test'

neg  = open('neg.txt','w')

def write_file(neg, path, num):
    
    filelist = glob('%s/*.jpg'%path)
    if len(filelist) < num:
        for f in filelist:
            neg.write('%s\n'%f)
        print '%s generate %d neg data'%(path, len(filelist))
    else:
        count = len(filelist)/num
        i = 0
        j = 0 
        for f in filelist:
            i+=1
            if i%count == 0:
                neg.write('%s\n'%f) 
                j+=1

        print '%s generate %d neg data'%(path, j)

#write_file(neg,VOCPATH,20000)
write_file(neg,NEGPATH,20000)
#write_file(neg,FACEPATH,40000)
#write_file(neg,CARPATH,30000)

