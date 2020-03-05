#include <cassert>
#include <iostream>

using namespace std;

template <typename T>
class Singleton
{
private:
    static T *m_Instance;

public:
    Singleton()
    {
        assert(m_Instance == nullptr);
        m_Instance = static_cast<T *>(this);
    }
    virtual ~Singleton()
    {
        m_Instance = nullptr;
    }
    static T &GetSingleton()
    {
        return *m_Instance;
    }
    static T *GetSingletonPtr()
    {
        return m_Instance;
    }
};

template <typename T>
T *Singleton<T>::m_Instance = nullptr;