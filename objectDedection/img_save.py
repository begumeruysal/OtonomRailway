import cv2
import numpy as np
import urllib.request
import os

# Dinamik IP adresi
ip = "192.168.5.87"

# ESP32 kamera IP adresi
esp32_ip = f"http://{ip}/cam-hi.jpg"

# Kareleri kaydedeceğimiz klasör
output_folder = "captured_frames"

# Klasör yoksa oluştur
if not os.path.exists(output_folder):
    os.makedirs(output_folder)

frame_count = 0

while True:
    try:
        # Kameradan görüntüyü al
        img_resp = urllib.request.urlopen(esp32_ip)
        img_np = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        img = cv2.imdecode(img_np, -1)

        if img is not None:
            # Görüntüyü ekranda göster
            cv2.imshow('ESP32 Camera', img)

            # Görüntüyü dosyaya kaydet
            frame_filename = os.path.join(output_folder, f"frame_{frame_count:04d}.jpg")
            cv2.imwrite(frame_filename, img)
            frame_count += 1

            # 'q' tuşuna basıldığında döngüyü sonlandır
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        else:
            print("Görüntü alınamadı, IP adresini ve bağlantıyı kontrol edin.")
            break
    except Exception as e:
        print(f"Bir hata oluştu: {e}")
        break

# Kaynakları serbest bırak
cv2.destroyAllWindows()
