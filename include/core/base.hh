#pragma once

namespace kretash {

class           Base {
public:
	Base() {}
	~Base() {}

	virtual void  init() {}
	virtual void  prepare() {}
	virtual void  update() {}
	virtual void  shutdown() {}
};
}