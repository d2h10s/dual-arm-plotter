# Dual_Arm_Plotter
## Brief
이 프로젝트는 Palleral Scara Robot을 간단한 기구학만으로 구현한 것입니다.  
서울과학기술대학교의 임베디드 시스템 프로그래밍 수업의 텀 프로젝트의 일환으로 제작되었습니다.
## 사용 부품
|항목|품명|버전|
|---|---|---|
|보드|arm cortex-m nucleo| F401RE|
|펌웨어|mbed-os|2.0|
|모터|GYEMS DM50 2개|-|
</br>

## 결함
이 프로젝트에 사용된 모터는 RS485 통신으로 PI 제어가 가능한 중국 GYEMS사의 DM50 입니다.  
모터 결함으로 인하여 송신은 가능하지만, 수신이 불가능합니다.  
이 프로젝트 이후 GEYEMS사의 최신의 최고가 모터인 RMD-X8-PRO 모델을 구매하였지만 중고 모터 배송 및 드라이버 결함이 똑같이 발견되어 폐기하였습니다.  

nucleo board에 쓰인 펌웨어는 mbed-os 2.0를 사용하였습니다.  
구버전이기 때문에 rs485, i2c를 포함한 serial 통신에 결함이 있기 때문에 송수신 전후에 10us~50us의 딜레이를 주지 않으면 통신 신호를 캐치하지 못합니다.  
최신의 OS에서 해결되었는지는 알 수 없지만 유념하시기 바랍니다.  
## 실물
![img](img/overall.jpg)
![img](img/detail.jpg)
[Go to Youtube](https://www.youtube.com/watch?v=BTTwyRU6h7Y)
## 회로도
![img](img/schematic.png)
