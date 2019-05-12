template <typename T>
struct ValueChangeTrigger {
	T value;
	bool changed;

	ValueChangeTrigger(T initialValue) : value(initialValue), changed(false) { }

	bool process(T newValue) {
		changed = value != newValue;
		value = newValue;
		return changed;
	}
};
