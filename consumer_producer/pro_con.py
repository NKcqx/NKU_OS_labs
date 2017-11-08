import threading,time
count = 5
Max = 5
def produce(con):
    global count,Max
    while True:
        con.acquire()
        if(count == 1):
            con.notify()
        while(count >= Max):
            con.wait()
        count += 1
        print('produce one more item remain:', count)
        con.release()
        time.sleep(0.4)

def consume(con):
    global count
    while True:
        con.acquire()
        while (count <= 0):
            con.notify()
            con.wait()
        count -= 1
        print('consume one item, remain:', count)
        con.release()
        time.sleep(1.2)

cond = threading.Condition()
producer = threading.Thread(target=produce, args=(cond,))
consumer1 = threading.Thread(target=consume, args=(cond,))
consumer2 = threading.Thread(target=consume, args=(cond,))
consumer3 = threading.Thread(target=consume, args=(cond,))
producer.start()
consumer1.start()
#consumer2.start()
#consumer3.start()
