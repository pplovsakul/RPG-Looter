#ifndef _SIG_SIGSLOT_H_
#define _SIG_SIGSLOT_H_

#include <functional>
#include <vector>

namespace sig
{
	// Signal with no parameters
	class signal0
	{
	public:
		using slot_type = std::function<void()>;

		void connect(slot_type slot)
		{
			slots.push_back(slot);
		}

		void send()
		{
			for (auto& slot : slots)
			{
				if (slot)
					slot();
			}
		}

	private:
		std::vector<slot_type> slots;
	};

	// Signal with one parameter
	template<typename T1>
	class signal1
	{
	public:
		using slot_type = std::function<void(T1)>;

		void connect(slot_type slot)
		{
			slots.push_back(slot);
		}

		void send(T1 arg1)
		{
			for (auto& slot : slots)
			{
				if (slot)
					slot(arg1);
			}
		}

	private:
		std::vector<slot_type> slots;
	};

	// Signal with two parameters
	template<typename T1, typename T2>
	class signal2
	{
	public:
		using slot_type = std::function<void(T1, T2)>;

		void connect(slot_type slot)
		{
			slots.push_back(slot);
		}

		void send(T1 arg1, T2 arg2)
		{
			for (auto& slot : slots)
			{
				if (slot)
					slot(arg1, arg2);
			}
		}

	private:
		std::vector<slot_type> slots;
	};
}

#endif // _SIG_SIGSLOT_H_
