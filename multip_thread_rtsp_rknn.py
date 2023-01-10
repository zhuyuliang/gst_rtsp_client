#!/usr/bin/env python3.6
import os
import time
import threading
import cv2
import numpy as np

from multiprocessing import Lock, Process, Condition, Queue

import gst_rtsp_client_api as rtspclient

from rknnlite.api import RKNNLite

# from ctypes import *

GRID0 = 13
GRID1 = 26
GRID2 = 52
LISTSIZE = 8
SPAN = 3
NUM_CLS = 3
MAX_BOXES = 500
OBJ_THRESH = 0.9
NMS_THRESH = 0.2

CLASSES = ("phone","call_phone","play_phone")

def sigmoid(x):
    return 1 / (1 + np.exp(-x))


def process(input, mask, anchors):

    anchors = [anchors[i] for i in mask]
    grid_h, grid_w = map(int, input.shape[0:2])

    box_confidence = sigmoid(input[..., 4])
    box_confidence = np.expand_dims(box_confidence, axis=-1)

    box_class_probs = sigmoid(input[..., 5:])

    box_xy = sigmoid(input[..., :2])
    box_wh = np.exp(input[..., 2:4])
    box_wh = box_wh * anchors

    col = np.tile(np.arange(0, grid_w), grid_w).reshape(-1, grid_w)
    row = np.tile(np.arange(0, grid_h).reshape(-1, 1), grid_h)

    col = col.reshape(grid_h, grid_w, 1, 1).repeat(3, axis=-2)
    row = row.reshape(grid_h, grid_w, 1, 1).repeat(3, axis=-2)
    grid = np.concatenate((col, row), axis=-1)

    box_xy += grid
    box_xy /= (grid_w, grid_h)
    box_wh /= (416, 416)
    box_xy -= (box_wh / 2.)
    box = np.concatenate((box_xy, box_wh), axis=-1)

    return box, box_confidence, box_class_probs

def filter_boxes(boxes, box_confidences, box_class_probs):
    """Filter boxes with object threshold.

    # Arguments
        boxes: ndarray, boxes of objects.
        box_confidences: ndarray, confidences of objects.
        box_class_probs: ndarray, class_probs of objects.

    # Returns
        boxes: ndarray, filtered boxes.
        classes: ndarray, classes for boxes.
        scores: ndarray, scores for boxes.
    """
    box_scores = box_confidences * box_class_probs
    box_classes = np.argmax(box_scores, axis=-1)
    box_class_scores = np.max(box_scores, axis=-1)
    pos = np.where(box_class_scores >= OBJ_THRESH)

    boxes = boxes[pos]
    classes = box_classes[pos]
    scores = box_class_scores[pos]

    return boxes, classes, scores

def nms_boxes(boxes, scores):
    """Suppress non-maximal boxes.

    # Arguments
        boxes: ndarray, boxes of objects.
        scores: ndarray, scores of objects.

    # Returns
        keep: ndarray, index of effective boxes.
    """
    x = boxes[:, 0]
    y = boxes[:, 1]
    w = boxes[:, 2]
    h = boxes[:, 3]

    areas = w * h
    order = scores.argsort()[::-1]
    keep = []
    while order.size > 0:
        i = order[0]
        keep.append(i)

        xx1 = np.maximum(x[i], x[order[1:]])
        yy1 = np.maximum(y[i], y[order[1:]])
        xx2 = np.minimum(x[i] + w[i], x[order[1:]] + w[order[1:]])
        yy2 = np.minimum(y[i] + h[i], y[order[1:]] + h[order[1:]])

        w1 = np.maximum(0.0, xx2 - xx1 + 0.00001)
        h1 = np.maximum(0.0, yy2 - yy1 + 0.00001)
        inter = w1 * h1

        ovr = inter / (areas[i] + areas[order[1:]] - inter)
        inds = np.where(ovr <= NMS_THRESH)[0]
        order = order[inds + 1]
    keep = np.array(keep)
    return keep


def yolov3_post_process(input_data):
    # yolov3
    masks = [[6, 7, 8], [3, 4, 5], [0, 1, 2]]
    anchors = [[10, 13], [16, 30], [33, 23], [30, 61], [62, 45],
               [59, 119], [116, 90], [156, 198], [373, 326]]
    # yolov3-tiny
    # masks = [[3, 4, 5], [0, 1, 2]]
    # anchors = [[10, 14], [23, 27], [37, 58], [81, 82], [135, 169], [344, 319]]

    boxes, classes, scores = [], [], []
    for input,mask in zip(input_data, masks):
        b, c, s = process(input, mask, anchors)
        b, c, s = filter_boxes(b, c, s)
        boxes.append(b)
        classes.append(c)
        scores.append(s)

    boxes = np.concatenate(boxes)
    classes = np.concatenate(classes)
    scores = np.concatenate(scores)

    nboxes, nclasses, nscores = [], [], []
    for c in set(classes):
        inds = np.where(classes == c)
        b = boxes[inds]
        c = classes[inds]
        s = scores[inds]

        keep = nms_boxes(b, s)

        nboxes.append(b[keep])
        nclasses.append(c[keep])
        nscores.append(s[keep])

    if not nclasses and not nscores:
        return None, None, None

    boxes = np.concatenate(nboxes)
    classes = np.concatenate(nclasses)
    scores = np.concatenate(nscores)

    return boxes, classes, scores

def draw(image, boxes, scores, classes):
    """Draw the boxes on the image.

    # Argument:
        image: original image.
        boxes: ndarray, boxes of objects.
        classes: ndarray, classes of objects.
        scores: ndarray, scores of objects.
        all_classes: all classes name.
    """
    for box, score, cl in zip(boxes, scores, classes):
        x, y, w, h = box
        print('class: {}, score: {}'.format(CLASSES[cl], score))
        print('box coordinate left,top,right,down: [{}, {}, {}, {}]'.format(x, y, x+w, y+h))

        x *= image.shape[1]
        y *= image.shape[0] 
        w *= image.shape[1]
        h *= image.shape[0]
        print("image.shape[]"+str(image.shape[0]))
        print("image.shape[]"+str(image.shape[1]))
        top = max(0, np.floor(x + 0.5).astype(int))
        left = max(0, np.floor(y + 0.5).astype(int))
        right = min(image.shape[1], np.floor(x + w + 0.5).astype(int))
        bottom = min(image.shape[0], np.floor(y + h + 0.5).astype(int))

        cv2.rectangle(image, (top, left), (right, bottom), (255, 0, 0), 2)
        cv2.putText(image, '{0} {1:.2f}'.format(CLASSES[cl], score),
                    (top, left - 6),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6, (0, 0, 255), 2)

def download_yolov3_weight(dst_path):
    if os.path.exists(dst_path):
        print('yolov3.weight exist.')
        return
    print('Downloading yolov3.weights...')
    url = 'https://pjreddie.com/media/files/yolov3.weights'
    try:
        urllib.request.urlretrieve(url, dst_path)
    except urllib.error.HTTPError as e:
        print('HTTPError code: ', e.code)
        print('HTTPError reason: ', e.reason)
        exit(-1)
    except urllib.error.URLError as e:
        print('URLError reason: ', e.reason)
    else:
        print('Download yolov3.weight success.') 

def func_rtspdisplay(index,url, usr, pwd):

    width = 1920
    height = 1080

    # c_url = create_string_buffer(url.encode('utf-8'), len(url))
    # ret = rtspclient.createRtspClient(index,url)

    RKNN_MODEL_PATH = '/home/toybrick/Dev/gst_rtsp_client/yolov3_phone_q_p.rknn'

    rknn = RKNNLite()
    print('Loading RKNN model')
    ret = rknn.load_rknn(RKNN_MODEL_PATH)
    if ret != 0:
        print('load rknn model failed.')
        exit(ret)
    print('done')

    print('--> init runtime')
    ret = rknn.init_runtime()
    if ret != 0:
        # i
        print('init runtime failed.')
        exit(ret)
    print('done')

    last = time.time()
    
    while(1) :
        ret1 = rtspclient.isConnect(index)
        time.sleep(0.5)
        print("id %d ret = %d",index, ret1)
        if (ret1 == 1):
            now = time.time()
            ret2 = rtspclient.mread(index,width,height)
            print(ret2.status)
            # print(type(img))
            # img = cv2.resize(img,(1920,1080,3))
            # img = cv2.UMat(height,width,cv2.CV_8UC3,img)
            # inference
            print('--> inference')
            # #img = cv2.resize(frame,(416,416))
            
            img = ret2.frame
            img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            img = cv2.resize(img,(416,416))
            outputs = rknn.inference(inputs=[img])
            print('done')
        
            #print(len(outputs))
            #print(outputs[0].shape)

            input0_data = outputs[0]
            input1_data = outputs[1]
            input2_data = outputs[2]

            input0_data = input0_data.reshape(SPAN, LISTSIZE, GRID0, GRID0)
            input1_data = input1_data.reshape(SPAN, LISTSIZE, GRID1, GRID1)
            input2_data = input2_data.reshape(SPAN, LISTSIZE, GRID2, GRID2)

            input_data = []
            input_data.append(np.transpose(input0_data, (2, 3, 0, 1)))
            input_data.append(np.transpose(input1_data, (2, 3, 0, 1)))
            input_data.append(np.transpose(input2_data, (2, 3, 0, 1)))

            boxes, classes, scores = yolov3_post_process(input_data)
        
            if boxes is not None:
                print("phone")
            # draw(frame.array(), boxes, scores, classes)

            # gl.show(idx0, frame)
            # print (frame_index, "----------------------------",now - last)
            # frame = frame.array()
            #cv2.imwrite("images/4_" + str(frame_index) + ".jpg", frame)
            #cv2.imshow('Carplate demo', cv2.resize(frame, (960, 540)))  #
            # cv2.waitKey(1)
            # frame_index += 1
            last = now

            cv2.imwrite('a' + str(index) +'.jpg',ret2.frame)
        elif (ret1 == 2):
            #time.sleep(4)
            rtspclient.createRtspClient(index,url)
            #time.sleep(10)

    print('# End of Thread %d' % (index))


if __name__ == '__main__':
    os.system('iptables -F')  # Disable Firewall

    #gl = toy.output.createGLDrmDisplay(toy.DisplayPort.HDMI_A)
    #idx0 = gl.add_view(0, 180, 640, 360)
    # idx1 = gl.add_view(640, 180, 640, 360)
    # idx2 = gl.add_view(1280, 180, 640, 360)
    # idx3 = gl.add_view(0, 540, 640, 360)
    # idx4 = gl.add_view(640, 540, 640, 360)
    # idx5 = gl.add_view(1280, 540, 640, 360)

    t5 = threading.Thread(target=func_rtspdisplay, args = (6, "rtsp://admin:passwd@192.168.2.35/Streaming/Channels/1", "admin", "passwd"))

    t5.start()

    t5.join()
