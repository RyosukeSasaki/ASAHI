#define TWE_LITE_USE_HARDWARE_SERIAL
#include "../TWE-Lite/TWE-Lite.hpp"
#include "GPS/GPS.hpp"

// ボードレート
#define BRATE	38400

// 動作モード
enum class Mode : char {
	Wait,		// ウェイトモード．コマンド受領までなにもしないをする．
	Standby,	// スタンバイモード．離床判定を行う．
	Flight,		// フライトモード．離床〜開傘まで．
	Descent,	// ディセントモード．開傘〜着水まで．
};

//#define NO_GPS
//#define NO_TWE

// グローバル変数
Mode g_mode;
#ifndef NO_GPS
	GPS gps(5, 6);
#endif
#ifndef NO_TWE
	TWE_Lite twelite(0, 1, BRATE);
#endif

// 初期化関数．一度だけ実行される．
void setup(){
	Serial.begin(BRATE);

	//TODO: センサ初期化
#ifndef NO_GPS
	gps.init(BRATE);
	delay(1000);
#endif

	//TODO: TWE-Lite初期化
#ifndef NO_TWE
	twelite.init();
	twelite.send_buf[0] = 0x01;
#endif

	//TODO: 動作モードをSDカードから読み込む
	// (動作中に瞬断して再起動する可能性がある)
	if(g_mode != Mode::Standby)
		return;		// 再起動時は早くそのモードの動作に戻る
	//TODO: 地上局に起動を通知
}

void loop(){
	switch(g_mode){
		case Mode::Wait:
			break;
		case Mode::Standby:
			break;
		case Mode::Flight:
			break;
		case Mode::Descent:
			break;
	}

#ifndef NO_GPS
	Serial.print("GPS: ");
	for(size_t i=0;i<500;i++){
		const int c = gps.read();
		if(c >= 0)
			Serial.write((char)c);
	}
	Serial.println("");
#endif

#ifndef NO_TWE
	//TODO: テレメトリ送信
	if(twelite.send(0x78, 1)){
		Serial.println("TWE-Lite send success");
	}else{
		Serial.println("TWE-Lite send failed");
	}
#endif
	delay(300);
}
