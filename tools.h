#pragma once

class Timer
{
public:
	Timer()
	{
		m_start = clock();
	}

	float GetElapsedTime()
	{
		clock_t t = clock() - m_start;
		return (float)t / (float)CLOCKS_PER_SEC;
	}

private:
	clock_t m_start;
};



class SpinLock
{
	std::atomic_flag locked = ATOMIC_FLAG_INIT;

public:
	void lock()
	{
		while (locked.test_and_set(std::memory_order_acquire)) { ; }
	}

	void unlock()
	{
		locked.clear(std::memory_order_release);
	}
};
