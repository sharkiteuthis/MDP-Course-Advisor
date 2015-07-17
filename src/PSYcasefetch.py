#!/usr/bin/python

import MySQLdb

conn = MySQLdb.connect (host = "mysql",
			user = "advisor",
			passwd = "bifftannen",
			db = "advisor")
#acceptable grades
accgrades = ['A','B','C','D','E','W']
courses = ['PSY_100', 'PSY_215', 'PSY_216', 'PSY_311', 'PSY_312', 'PSY_313', 'PSY_314']
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


keylist = grades.keys()
keylist.sort()

for k in keylist:
	sid, cid, year, sem = k
	print sid, cid, year, sem, grades[k]

print len(keylist)

cursor.close()
conn.close()


