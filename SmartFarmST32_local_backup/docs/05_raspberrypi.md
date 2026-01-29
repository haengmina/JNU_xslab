# Raspberry Pi Manual

이 문서는 라즈베리파이 설정부터 InfluxDB 및 Grafana 설치, 연동까지의 과정을 안내합니다.

---

## 라즈베리파이 기본 설정

### 라즈베리파이 이미저 설치

1.  **다운로드:** [www.raspberrypi.com/software/](http://www.raspberrypi.com/software/)에서 리눅스용 이미저를 다운로드합니다. (예: `imager_2.0.4_amd_64.AppImage`)
2.  **실행:** 터미널에서 저장된 폴더로 이동하여 다음 명령어로 실행합니다.
    ```bash
    sudo ./imager_2.0.4_amd_64.AppImage
    ```

### Wifi 연결 및 OS 설치

1.  **SD 카드 연결:** SD 카드를 PC에 삽입하고, WSL 환경에서 `zadig.exe`를 사용하여 uSD 카드 리더 드라이버를 `libusb-win32`로 변경 후 WSL에 연결합니다.
2.  **OS 선택:** Raspberry Pi Imager에서 `Raspberry Pi OS Lite (64-bit)`를 선택합니다.
3.  **고급 옵션 설정:** **반드시** 다음 고급 옵션을 설정합니다.
    *   사용자 이름 / 비밀번호
    *   Wi-Fi SSID / 비밀번호
    *   국가: `KR`
    *   SSH 활성화 (비밀번호 인증 사용 체크)
    *   라즈베리 파이 커넥트 활성화 (웹 브라우저를 이용한 GUI 사용이 가능하기 때문에 선택적으로 활성화 )
4.  **설치 및 부팅:** `Write` 버튼을 클릭하여 SD 카드에 OS를 설치한 후, 라즈베리파이에 삽입하고 전원을 연결합니다.
5.  **IP 확인:** 1~2분 후, `arp -a` 명령어를 실행하여 새로 추가된 `dynamic` 타입의 IP 주소를 확인합니다.

### SSH 접속 및 기본 설정

1.  **SSH 접속:** Ubuntu에서 다음 명령어로 라즈베리파이에 접속합니다.
    ```bash
    ssh <user_name>@<라즈베리파이_ip주소>
    # 예시: ssh mini@192.168.0.57
    ```
2.  **시스템 업그레이드:**
    ```bash
    sudo apt update && sudo apt upgrade -y
    ```
3.  **파이썬 설치:**
    ```bash
    sudo apt-get install python3
    python --version
    ```
4.  **VSCode SSH 설정:** VSCode에서 `Remote-SSH` 확장팩을 설치하고, `~/.ssh/config` 파일에 다음 내용을 추가합니다.
    ```
    Host raspberrypi
        HostName 192.168.0.57
        User mini
    ```
    이후 `Remote-SSH: Connect to Host...`를 통해 `raspberrypi`에 접속합니다.

---

## Raspberry Pi Connect 연결 (GUI)

### 터미널을 이용한 연결

1.  **설치:** 라즈베리파이 터미널에서 다음 명령어를 실행합니다.
    ```bash
    sudo apt update
    sudo apt install rpi-connect
    ```
2.  **로그인:** `rpi-connect signin` 명령어를 실행하고, 웹 브라우저에서 Raspberry Pi 계정으로 로그인합니다.
3.  **GUI 사용:** Screen sharing 기능을 통해 GUI를 사용합니다.

### 웹 브라우저를 이용한 연결

1.  [raspberrypi.com/connect](http://raspberrypi.com/connect)에 접속하여 로그인합니다.
2.  Device 리스트에서 접속할 라즈베리파이의 `connect via`를 클릭하고, `Screen sharing`을 선택합니다.

---

## SD 카드 이미지 백업

### Windows 사용자

1.  **Win32 Disk Imager**를 설치하고 실행합니다.
2.  백업할 이미지 파일의 위치와 이름을 지정합니다.
3.  백업할 SD 카드의 드라이브 문자를 정확히 선택합니다. **(매우 중요)**
4.  `Read` 버튼을 클릭하여 백업을 시작합니다.

### Linux 사용자

1.  `lsblk` 또는 `sudo fdisk -l` 명령어로 SD 카드의 장치 이름을 확인합니다. (예: `/dev/sdb`)
2.  `dd` 명령어를 사용하여 백업합니다.
    *   **Linux 예시:** `sudo dd if=/dev/sdb of=~/Desktop/my-raspberry-pi.img bs=4M status=progress`
    *   **macOS 예시:** `sudo dd if=/dev/rdisk2 of=~/Desktop/my-raspberry-pi.img bs=4m`
        *   *(macOS에서는 `/dev/rdisk`를 사용하면 속도가 더 빠릅니다.)*

> **⚠️ 경고:** `if=`(입력)와 `of=`(출력)를 혼동하면 시스템 데이터가 손상될 수 있습니다.

---

## InfluxDB 및 Grafana 설치

### InfluxDB 설치

1.  **GPG 키 및 저장소 추가:** 다음 명령어를 실행하여 InfluxData의 GPG 키를 다운로드하고 저장소를 추가합니다.
    ```bash
    curl --silent --location -O https://repos.influxdata.com/influxdata-archive.key

    gpg --show-keys --with-fingerprint --with-colons ./influxdata-archive.key 2>&1 | grep -q '^fpr:\+24C975CBA61A024EE1B631787C3D57159FC2F927:$' && \cat influxdata-archive.key | gpg --dearmor | sudo tee /etc/apt/keyrings/influxdata-archive.gpg > /dev/null && \echo 'deb [signed-by=/etc/apt/keyrings/influxdata-archive.gpg] https://repos.influxdata.com/debian stable main' | sudo tee /etc/apt/sources.list.d/influxdata.list
    ```
2.  **InfluxDB 설치:**
    ```bash
    sudo apt-get update
    sudo apt-get install influxdb2 influxdb2-cli -y
    ```
3.  **서비스 시작:**
    ```bash
    sudo systemctl start influxdb
    sudo systemctl enable influxdb
    sudo systemctl status influxdb
    ```

### Grafana 설치

1.  **필수 패키지 설치:**
    ```bash
    sudo apt-get update
    sudo apt-get install -y apt-transport-https software-properties-common wget
    ```
2.  **GPG 키 및 저장소 추가:**
    ```bash
    sudo mkdir -p /etc/apt/keyrings/

    wget -q -O - https://apt.grafana.com/gpg.key | gpg --dearmor | sudo tee /etc/apt/keyrings/grafana.gpg > /dev/null
    
    echo "deb [signed-by=/etc/apt/keyrings/grafana.gpg] https://apt.grafana.com stable main" | sudo tee /etc/apt/sources.list.d/grafana.list
    ```
3.  **Grafana 설치:**
    ```bash
    sudo apt-get update
    sudo apt-get install grafana
    ```
4.  **서비스 시작:**
    ```bash
    sudo systemctl daemon-reload
    sudo systemctl start grafana-server
    sudo systemctl enable grafana-server.service
    ```
    *`sudo systemctl status grafana-server` 명령으로 `active (running)` 상태를 확인합니다.*

---

## Grafana에서 InfluxDB 연동

### Grafana 접속

*   웹 브라우저에서 `http://<Raspberry Pi IP 주소>:3000`으로 접속합니다.
*   초기 아이디: `admin`, 비밀번호: `admin`

### 데이터 소스 추가

1.  왼쪽 메뉴의 **톱니바퀴 아이콘** > **Administration** > **Plugins and data** > **Plugins**로 이동하여 `InfluxDB`를 선택합니다.
2.  다음 정보를 입력하여 데이터 소스를 설정합니다.
    *   **Name:** 데이터 소스 이름 (예: `Local InfluxDB`)
    *   **Query Language:** `Flux`
    *   **URL:** `http://localhost:8086`
    *   **Organization:** InfluxDB 설정 시 생성한 조직 이름 (예: `JNU`)
    *   **Token:** InfluxDB에서 생성한 API 토큰 (읽기 권한 필요)
    *   **Default Bucket:** 데이터를 저장할 버킷 이름 (예: `SmartFarm`)
3.  `Save & test` 버튼을 클릭하여 연결을 확인합니다.

### 대시보드 생성

1.  왼쪽 메뉴의 **대시보드 아이콘** > **Create dashboard**를 선택합니다.
2.  **패널 설정:**
    *   **Data source:** 위에서 추가한 InfluxDB 데이터 소스를 선택합니다.
    *   **Query:** Flux 쿼리를 작성하여 시각화할 데이터를 가져옵니다.
        ```flux
        from(bucket: "your_bucket_name")
          |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
          |> filter(fn: (r) => r._measurement == "sensor_data")
          |> yield(name: "mean_values")
        ```
    *   **Visualization:** `Time series`와 같은 시각화 유형을 선택합니다.
3.  `Apply` 버튼으로 패널을 추가하고, 상단의 `Save` 버튼으로 대시보드를 저장합니다.
