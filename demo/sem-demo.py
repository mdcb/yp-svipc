#! /usr/bin/env python

import sys, os
from os import getpid, fork
from time import sleep, time
from svipc import sem_init, sem_take, sem_give, sem_cleanup

# create id
sid = 0x7dcb0000 | getpid()

# create a pool with one semaphore
sem_init(sid,nums=1)

# time unit for test
delay=0.5

class unit_test(object):
  def __init__(self):
    self.errmsg=''
    if fork():
      if not self.parent(): print 'OK'
      else: print 'ERROR', self.errmsg
    else:
      self.child()
      exit()
  def parent(self): return 0
  def child(self): pass

class ut1(unit_test):
  def parent(self):
    print '- wait for child forever:'
    s = sem_take(sid, 0)
    return s
  def child(self):
    sleep(delay)
    sem_give(sid,0)

class ut2(unit_test):
  def parent(self):
    print '- wait for child with timeout:'
    tstart=time()
    s = sem_take(sid, 0, wait= 2 * delay)
    if time()-tstart < delay:
      self.errmsg='timeout too quickly.'
      return -2
    return s
  def child(self):
    sleep(delay)
    sem_give(sid,0)

class ut3(unit_test):
  def parent(self):
    print '- wait for child with timeout, and timeout:'
    tstart=time()
    s = sem_take(sid, 0, wait=delay)
    if time()-tstart < delay:
      self.errmsg='did not wait long enough'
      return -2
    self.errmsg='wait did not time out.'
    return not s
  def child(self):
    pass

class ut4(unit_test):
  def parent(self):
    print '- wait for child x10 times:'
    s = sem_take(sid, 0, count=10)
    return s
  def child(self):
    for i in range(10):
      sem_give(sid,0)

class ut5(unit_test):
  def parent(self):
    print '- try take semaphore (available):'
    sleep(delay)
    s = sem_take(sid, 0, wait=0)
    self.errmsg='sem was not available'
    return s 
  def child(self):
    sem_give(sid,0)

class ut6(unit_test):
  def parent(self):
    print '- try take semaphore (non-available):'
    s = sem_take(sid, 0, wait=0)
    self.errmsg='sem was available'
    return not s 
  def child(self):
    pass


ut1()
ut2()
ut3()
ut4()
ut5()
ut6()
sem_cleanup(sid)
