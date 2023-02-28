#pragma once

#include <deque>
#include <functional>

class DeletionQueue
{
public:
	void push(std::function<void()>&& function)
	{
		deletion_queue.push_back(function);
	}

	void flush()
	{
		auto it = deletion_queue.rbegin();
		const auto end = deletion_queue.rend();
		// Iterate from back to front
		while (it != end)
		{
			// Call the deletor function
			(*it)();
			it++;
		}
		deletion_queue.clear();
	}

private:
	std::deque<std::function<void()>> deletion_queue;
};

