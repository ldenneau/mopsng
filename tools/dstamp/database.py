#!/usr/bin/env python

import MySQLdb

##################################################
class Database:
    def __init__(self, host, user, passwd, db):
        self.db = MySQLdb.connect(host = host,
                                  user = user, passwd = passwd,
                                  db = db)
    def select(self, stmt):
        cursor = self.db.cursor()
        cursor.execute(stmt)
        products = cursor.fetchall()
        cursor.close()
        return products

    def selectFirst(self, stmt):
        cursor = self.db.cursor()
        cursor.execute(stmt)
        products = cursor.fetchall()
        cursor.close()
        return products[0]
