# coding:utf-8
# 服务端
import os
import socket
import threading
import time

# 接收的方法
# def clientThreadIn(conn, nick):
#     while True:
#         try:
#             temp = conn.recv(1024)
#             if not temp:
#                 conn.close()
#                 return
#             print("temp len = ", len(temp))
#         except:
#             conn.close()
#             print(nick + 'leaves the room!')  # 出现异常就退出
#             return


# # 发送的方法
# def clientThreadOut(conn, nick):
#     global data
#     while True:
#         if con.acquire():
#             con.wait()  # 放弃对资源占有 等待通知
#             if data:
#                 try:
#                     conn.send(data)
#                     con.release()
#                 except:
#                     con.release()
#                     return


# def NotifyAll(ss):
#     global data
#     if con.acquire():  # 获取锁  原子操作
#         data = ss
#         con.notifyAll()  # 当前线程放弃对资源占有, 通知所有
#         con.release()



if __name__ == '__main__':

    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    conn = './temp/conn'
    if not os.path.exists(conn):
        os.mknod(conn)

    if os.path.exists(conn):
        os.unlink(conn)
        sock.bind(conn)
        sock.listen(1)

    while True:
        conn, addr = sock.accept()  # 接收连接
        # print('Connected with' + addr[0] + ':' + str(addr[1]))
        nick = conn.recv(1024)  # 获取用户名
        print('Welcome' + str(nick) + 'to the room')
        # print(data)
        # print(str((threading.activeCount() + 1) / 2) + 'person(s)')
        # conn.send(data)
        # threading.Thread(target=clientThreadIn, args=(conn, nick)).start()
        # threading.Thread(target=clientThreadOut, args=(conn, nick)).start()
        while True:
            try:
                time.sleep(2)
                conn.send('start'.encode('utf-8'))
                temp = ''
                while True:
                    temp = conn.recv(1024)
                    print(temp.decode('utf-8'))
                    if len(temp) != 0:break
                print("temp len = ", len(temp))
            except:
                conn.close()
                print(str(nick) + 'leaves the room!')  # 出现异常就退出
                break
