namespace twelite {
	struct vec_t {
		uint32_t time;
		float x, y, z;
	};

	extern vec_t latest_acc;
	extern vec_t latest_gyro;

	bool init();
	void loop();
}
