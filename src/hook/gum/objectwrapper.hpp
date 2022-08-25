#pragma once

template <typename T>
class GumObjectWrapper {
 public:
  GumObjectWrapper(T* obj, bool obtain_ref = false) : obj(obj) {
    if (obtain_ref)
      gum_object_ref(obj);
  };
  ~GumObjectWrapper() { gum_object_unref(obj); };

 protected:
  T* get_obj() { return obj; };

 private:
  T* obj;
};
