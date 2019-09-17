#pragma once
#include <queue>
#include <memory>  
#include <mutex>  
#include <condition_variable>  

namespace dji
{
	namespace videoparser
	{
		template<typename T>
		class threadsafe_queue
		{

		private:
			mutable std::mutex m_mut;
			std::queue<T> m_data_queue;
			std::condition_variable m_data_cond;

		public:
			threadsafe_queue() {}

			threadsafe_queue(threadsafe_queue const& other)
			{
				std::lock_guard<std::mutex> lk(other.m_mut);
				m_data_queue = other.m_data_queue;
			}

			void push(const T &new_value)
			{
				{
					std::lock_guard<std::mutex> lk(m_mut);
					m_data_queue.push(new_value);
				}
				m_data_cond.notify_one();
			}

			void push(T && new_value)
			{
				{
					std::lock_guard<std::mutex> lk(m_mut);
					m_data_queue.push(std::forward<T>(new_value));
				}
				m_data_cond.notify_one();
			}

			void wait_and_pop(T& value)
			{
				std::unique_lock<std::mutex> lk(m_mut);
				m_data_cond.wait(lk, [this] {return !m_data_queue.empty(); });
				value = std::move(m_data_queue.front());
				m_data_queue.pop();
			}

			template <class _Predicate> bool wait_for_item(_Predicate _Pred)
			{
				std::unique_lock<std::mutex> lk(m_mut);
				m_data_cond.wait(lk, [this, _Pred] {return (!m_data_queue.empty() && _Pred()); });

				return !m_data_queue.empty();
			}

			std::shared_ptr<T> wait_and_pop()
			{
				std::unique_lock<std::mutex> lk(m_mut);
				m_data_cond.wait(lk, [this] {return !m_data_queue.empty(); });
				std::shared_ptr<T> res = std::make_shared<T>(std::move(m_data_queue.front()));
				m_data_queue.pop();
				return res;
			}

			bool try_pop(T& value)
			{
				std::lock_guard<std::mutex> lk(m_mut);
				if (m_data_queue.empty())
					return false;
				value = std::move(m_data_queue.front());
				m_data_queue.pop();
				return true;
			}

			std::shared_ptr<T> front()
			{
				std::lock_guard<std::mutex> lk(m_mut);
				if (m_data_queue.empty())
					return nullptr;
				std::shared_ptr<T> res = std::make_shared<T>(m_data_queue.front());
				return res;
			}

			std::shared_ptr<T> try_pop()
			{
				std::lock_guard<std::mutex> lk(m_mut);
				if (m_data_queue.empty())
					return nullptr;
				std::shared_ptr<T> res = std::make_shared<T>(std::move(m_data_queue.front()));
				m_data_queue.pop();
				return res;
			}

			bool empty() const
			{
				std::lock_guard<std::mutex> lk(m_mut);
				return m_data_queue.empty();
			}

			size_t size() const
			{
				std::lock_guard<std::mutex> lk(m_mut);
				return m_data_queue.size();
			}

			void clear()
			{
				std::lock_guard<std::mutex> lk(m_mut);
				std::queue<T> tmp;
				std::swap(m_data_queue, tmp);
			}
		};

	}
}
