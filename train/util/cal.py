#!/usr/bin/env python
from PIL import Image
import os
import sys
import operator

FILE = 'log/ssd_log.txt'
FILE1 = 'log/yolo_log.txt'



def prepro_line(_line):
	l1 = _line.split(' ')
	l2 = []

	for i in l1:
		if i != '' and i != '\n':
			l2.append(i)
	return l2

def overlap(x1,w1,x2,w2):
	l1 = float(x1) - float(w1)*0.5
	l2 = float(x2) - float(w2)*0.5
	if l1 > l2:
		left = l1
	else:
		left = l2
	
	r1 = float(x1) + float(w1)/2	
	r2 = float(x2) + float(w2)/2
	if r1 < r2: 
		right = r1
	else: 
		right = r2
	
	return float(right - left) 

def intersection(l1,l2):
	w = overlap(l1[1],l1[3],l2[1],l2[3]);
	h = overlap(l1[2],l1[4],l2[2],l2[4]);
	if w < 0 or h < 0:
		return 0
	area = w * h
	return float(area)


def union(l1,l2):
	a = intersection(l1,l2)
	b = float(l1[3])*float(l1[4])+float(l2[3])*float(l2[4])-a;
	return b



def cal_iou(l_true,l_predict):
	return intersection(l_true,l_predict)/union(l_true,l_predict) 


def cal(true_path,l,iou_set):
	
	lines = open(true_path).readlines()
	lt = []
	for line in lines:
		l1 = prepro_line(line)
		lt.append(l1)
	lt.sort(key = operator.itemgetter(1,2))
	
	iou = 0.0
	recall = 0
	total_true = len(lt)
	total_pre = len(l)
	for i in lt :
		for j in l:
			if cal_iou(i,j) > iou_set:	
					recall = recall + 1	
					iou = iou + cal_iou(i,j)

	return [iou,recall,total_true,total_pre]

