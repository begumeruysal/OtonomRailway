import cv2
import requests
import numpy as np
import time

# Nesne tespiti için eşik değeri
thres = 0.50
# Dinamik IP adresi
ip = "192.168.5.87"

# ESP32 kamera IP adresi
esp32_ip = f"http://{ip}/cam-hi.jpg"

# ESP32 pin kontrol URL'leri
esp32_high_url = f"http://{ip}/gpio-high"
esp32_low_url = f"http://{ip}/gpio-low"

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

        if len(classIds) != 0:
            for classId, confidence, box in zip(classIds.flatten(), confs.flatten(), bbox):
                if classId <= len(classNames):  # classId'nin geçerli aralıkta olup olmadığını kontrol et
                    # Sınıf ismini ve güven oranını yaz
                    cv2.putText(img, classNames[classId - 1].upper(), (box[0] + 10, box[1] + 30),
                                cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 2)
                    cv2.putText(img, str(round(confidence * 100, 2)), (box[0] + 200, box[1] + 30),
                                cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 2)

                    # Trafik lambası tespiti
                    if classNames[classId - 1] == "traffic light":
                        # Tespit edilen bölgeyi HSV renk alanına çevir
                        hsv_img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
                        detected_color = hsv_img[box[1]:box[1]+box[3], box[0]:box[0]+box[2]]

                        # Kırmızı ve yeşil renk için maske oluşturma
                        lower_red1 = np.array([0, 100, 100])
                        upper_red1 = np.array([10, 255, 255])
                        lower_red2 = np.array([160, 100, 100])
                        upper_red2 = np.array([180, 255, 255])
                        mask_red1 = cv2.inRange(detected_color, lower_red1, upper_red1)
                        mask_red2 = cv2.inRange(detected_color, lower_red2, upper_red2)
                        mask_red = cv2.bitwise_or(mask_red1, mask_red2)

                        lower_green = np.array([35, 100, 100])
                        upper_green = np.array([85, 255, 255])
                        mask_green = cv2.inRange(detected_color, lower_green, upper_green)

                        # Kırmızı ve yeşil bölgeleri tespit et
                        red_detected = cv2.countNonZero(mask_red) > 0
                        green_detected = cv2.countNonZero(mask_green) > 0

                        if red_detected:
                            requests.get(esp32_high_url)
                            print("Kırmızı algılandı")
                            cv2.rectangle(img, box, color=(0, 0, 255), thickness=2)  # Kırmızı çerçeve ekle
                        elif green_detected:
                            requests.get(esp32_low_url)
                            print("Yeşil algılandı")
                            cv2.rectangle(img, box, color=(0, 255, 0), thickness=2)  # Yeşil çerçeve ekle
                        else:
                            cv2.rectangle(img, box, color=(255, 255, 0), thickness=2)  # Diğer renkler için sarı çerçeve
                    else:
                        # Diğer nesneler için sadece çerçeve ekle
                        cv2.rectangle(img, box, color=(255, 0, 0), thickness=2)

        cv2.imshow("Output", img)  # Görüntüyü göster
        if cv2.waitKey(1) & 0xFF == ord('q'):  # 'q' tuşuna basarak çıkma
            break

    except Exception as e:
        print(f"Görüntü alınamadı: {e}")
        time.sleep(1)  # 1 saniye bekle ve tekrar dene

cv2.destroyAllWindows()
