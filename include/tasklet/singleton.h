#ifndef SINGLETON_HH_INCLUDED
#define SINGLETON_HH_INCLUDED

template<class T>
class singleton
{
  static T *mObj;

 public:
  static T& instance(){
    if( mObj == NULL ) mObj = new T();
    return *mObj;
  }
};

template <typename T> T* singleton<T>::mObj = NULL;

#endif // SINGLETON_HH_INCLUDED
