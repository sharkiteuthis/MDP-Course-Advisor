#!/usr/bin/python

import MySQLdb

conn = MySQLdb.connect (host = "mysql",
			user = "advisor",
			passwd = "bifftannen",
			db = "advisor")
#acceptable grades
accgrades = ['A','B','C','D','E','W']
courses = ['CS_115', 'CS_215', 'CS_216', 'CS_275', 'CS_315']
cursor = conn.cursor()
grades = {}

for x in courses:
	cursor.execute("select Sid,Cid,year,sem,grade from transcripts where Cid=%s;", x)

#create dictionary for each Sid
	result = cursor.fetchall()

#grades based on key/value pairs


#iterate
	for rec in result:
		if str(rec[4]) in accgrades:
			indextup = rec[0],rec[1], rec[2], rec[3]
			grades[indextup] = rec[4]	

#do a bastard join with MA114

cursor.execute("select Sid,Cid,year,sem,grade from transcripts where Cid=\"MA_114\";")
result = cursor.fetchall()

gradekeys = grades.keys()
sids = []

for k in gradekeys:
	sid, cid, year, sem = k
	if sid not in sids:
		sids.append(sid)

for rec in result:
	if str(rec[4]) in accgrades:
		if rec[0] in sids:
			indextup = rec[0],rec[1], rec[2], rec[3]
			grades[indextup] = rec[4]	


#only insert them into the dictionary if they took another course
#ie. their Sid appears in the dict...



keylist = grades.keys()
keylist.sort()

distsid = 0
lastsid = -1

for k in keylist:
	sid, cid, year, sem = k
	if(sid != lastsid):
		distsid+=1
		lastsid = sid
	print sid, cid, year, sem, grades[k]

print distsid

cursor.close()
conn.close()


