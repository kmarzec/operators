#pragma once

class timer
{
public:
	timer()
	{
		m_start = clock();
	}

	float get_elapsed_time()
	{
		clock_t t = clock() - m_start;
		return (float)t / (float)CLOCKS_PER_SEC;
	}

private:
	clock_t m_start;
};



class spin_lock
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
