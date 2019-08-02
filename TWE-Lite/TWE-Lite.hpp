#ifndef TWE_LITE_H_
#define TWE_LITE_H_

#ifdef ARDUINO
	// Arduino
	#include <SoftwareSerial.h>
#else
	// Linux
	#include <cstdio>
	#include <string>
	#ifdef _WIN32
		#error fuck Windows
	#elif defined(RASPBERRY_PI)
		// Raspberry Pi
		#include <wiringPi.h>
		#include <wiringSerial.h>
	#endif
#endif

class TWE_Lite {
public:
	// コンパイル時定数
	constexpr static long int default_brate	= 115200;
	constexpr static uint8_t buf_size		= 80;
	constexpr static uint16_t MSB			= 0x8000;

	// コンストラクタ
#ifdef ARDUINO
	explicit TWE_Lite(const uint8_t rx, const uint8_t tx, const long int &brate) : rx(rx), tx(tx), brate(brate) {}
	explicit TWE_Lite(const uint8_t rx, const uint8_t tx) : rx(rx), tx(tx), brate(default_brate) {}
#else
	explicit TWE_Lite(const std::string &devfile, const long int &brate) : devfile(devfile), brate(brate) {}
#endif

	// デストラクタ
	~TWE_Lite(){
#ifdef ARDUINO
		// 特にやることなし(そもそもデストラクタが呼ばれるべきではない)
#else
	if(fd != 0){
	#ifdef RASPBERRY_PI
		serialFlush(fd);
		serialClose(fd);
	#else
		std::fclose(fd);
	#endif
	}
#endif
	}

	// 定数
#ifdef ARDUINO
	const uint8_t rx, tx;
#else
	const std::string devfile;
#endif
	const long int brate;

	// publicな変数
	uint8_t send_buf[buf_size];
	uint8_t recv_buf[buf_size];

	// 初期化
	bool init(){
		parser.set_buf(recv_buf);
#ifdef RASPBERRY_PI
		fd = serialOpen(devfile.c_str(), brate);
		if(fd < 0) return false;
#elif ARDUINO
		serial = new SoftwareSerial(rx, tx);
		serial->begin(brate);
#elif RASPBERRY_PI
		fd = serialOpen(devfile.c_str(), brate)
#endif

		return true;
	}

	// シリアル通信の抽象化
	inline void swrite8(const uint8_t &val){
#ifdef ARDUINO
		serial->write(val);
#endif
	}
	inline void swrite16_big(const uint16_t &val){
		swrite8(static_cast<uint8_t>(val >> 8));
		swrite8(static_cast<uint8_t>(val & 0xff));
	}
	inline void swrite(const uint8_t *buf, const size_t &size){
#ifdef ARDUINO
		serial->write(buf, size);
#endif
	}

	inline uint8_t sread8(){
#ifdef ARDUINO
		return serial->read();
#elif defined(RASPBERRY_PI)
		return serialGetchar(fd);
#endif
	}

	inline int savail() const {
#ifdef ARDUINO
		return serial->available();
#elif defined(RASPBERRY_PI)
		return serialDataAvail(fd);
#endif
	}

	// 現在のバッファを送信
	bool send(const uint8_t &size){
		uint8_t header[] = { 0xA5, 0x5A }; // binary mode
		uint8_t id = 0x78;
		uint8_t cmd_type = 0x01;

		// 送信コマンド長
		uint16_t cmd_size = MSB + static_cast<uint16_t>(size) + 2; // 簡易形式

		// checksum
		uint8_t checksum = id ^ cmd_type;
		for(uint8_t i=0;i<size;i++)
			checksum = checksum ^ send_buf[i];

		// 送信
		swrite(header, 2);
		swrite16_big(cmd_size);
		swrite8(id);
		swrite8(cmd_type);
		swrite(send_buf, size);
		swrite8(checksum);

		return true;
	}

	size_t recv(size_t timeout=0){
		while(true){
			if(parser.parse8(sread8()))
				break;
		}
		return parser.get_length();
	}

	// binary mode parser
	class Parser {
	public:
		explicit Parser() : s(state::empty), length(0x0000), checksum(0x00) {}

		enum class state : uint8_t {
			empty,
			header,
			length,
			length2,
			payload,
			checksum,
		};

		inline auto get_state() const -> state { return s; }
		inline auto get_error() const -> state { return e; }
		inline auto get_length() const -> const uint16_t { return length; }

		inline void set_buf(uint8_t *buf){
			payload = buf;
		}

		bool parse8(const uint8_t &b){
			switch(s){
			case state::empty:
				e = state::empty;
				if(b == 0xA5)
					s = state::header;
				break;
			case state::header:
				if(b == 0x5A)
					s = state::length;
				else
					error();
				break;
			case state::length:
				if(b & 0x80){
					length = (b & 0x7F) << 8;
					s = state::length2;
				}else{
					// short length mode(?)
					error();
				}
				break;
			case state::length2:
				length += b;
				if(length <= buf_size)
					s = state::payload;
				else
					error();
				break;
			case state::payload:
				payload[pos] = b;
				checksum ^= b;
				if(pos == length-1)
					s = state::checksum;
				pos++;
				break;
			case state::checksum:
				if(b == checksum){
					s = state::empty;
					pos = 0;
					checksum = 0x00;
					return true;
				}else
					error();
				break;
			}
			return false;
		}
	private:
		state s, e;
		uint16_t length; // 送信コマンドの長さ
		uint8_t checksum;
		uint16_t pos;
		uint8_t *payload;

		inline void error(){
			e = s;
			s = state::empty;
		}
	} parser;

private:
#ifdef ARDUINO
	SoftwareSerial *serial = nullptr;
#else
	int fd = 0;
#endif
};

#endif
