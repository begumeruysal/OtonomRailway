import cv2
import requests
import numpy as np
import time

# Nesne tespiti için eşik değeri
thres = 0.45

# ESP32 kamera IP adresi
esp32_ip = "http://192.168.5.87/cam-hi.jpg"

# ESP32 pin kontrol URL'leri
esp32_high_url = "http://192.168.5.87/gpio-high"
esp32_low_url = "http://192.168.5.87/gpio-low"

# coco.names dosyasından sınıf isimlerini yükleme
classNames = []
classFile = 'coco.names'
with open(classFile, 'rt') as f:
    classNames = f.read().rstrip('\n').split('\n')

# Yapılandırma ve ağırlık dosyalarını yükleme
configPath = 'ssd_mobilenet_v3_large_coco_2020_01_14.pbtxt'
weightsPath = 'frozen_inference_graph.pb'

# Sinir ağını başlatma
net = cv2.dnn_DetectionModel(weightsPath, configPath)
net.setInputSize(320, 320)
net.setInputScale(1.0 / 127.5)
net.setInputMean((127.5, 127.5, 127.5))
net.setInputSwapRB(True)

while True:
    try:
        # ESP32 kamera HTTP'den görüntü alma
        response = requests.get(esp32_ip)
        img_array = np.array(bytearray(response.content), dtype=np.uint8)
        img = cv2.imdecode(img_array, -1)

        # Nesne tespiti
        classIds, confs, bbox = net.detect(img, confThreshold=thres)

        stop_sign_detected = False

        if len(classIds) != 0:
            for classId, confidence, box in zip(classIds.flatten(), confs.flatten(), bbox):
                if classId <= len(classNames):  # classId'nin geçerli aralıkta olup olmadığını kontrol et
                    # Sınıf ismini ve güven oranını yaz
                    cv2.putText(img, classNames[classId - 1].upper(), (box[0] + 10, box[1] + 30),
                                cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 2)
                    cv2.putText(img, str(round(confidence * 100, 2)), (box[0] + 200, box[1] + 30),
                                cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 2)

                    # Stop sign tespiti
                    if classNames[classId - 1] == "stop sign":
                        stop_sign_detected = True
                        cv2.rectangle(img, box, color=(0, 0, 255), thickness=2)  # Kırmızı çerçeve ekle

                    # Diğer nesneler için sadece çerçeve ekle
                    else:
                        cv2.rectangle(img, box, color=(255, 0, 0), thickness=2)

        # Stop sign tespit edildiyse HIGH sinyali gönder, aksi takdirde LOW sinyali gönder
        if stop_sign_detected:
            requests.get(esp32_high_url)
            print("Stop sign algılandı")
        else:
            requests.get(esp32_low_url)
            print("Stop sign algılanmadı")

        cv2.imshow("Output", img)  # Görüntüyü göster
        if cv2.waitKey(1) & 0xFF == ord('q'):  # 'q' tuşuna basarak çıkma
            break

    except Exception as e:
        print(f"Görüntü alınamadı: {e}")
        time.sleep(1)  # 1 saniye bekle ve tekrar dene

cv2.destroyAllWindows()
