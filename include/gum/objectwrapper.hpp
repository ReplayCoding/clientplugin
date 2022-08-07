#pragma once

template <typename T>
class GumObjectWrapper {
 public:
  GumObjectWrapper(auto obj, bool obtain_ref = false) : obj(obj) {
    if (obtain_ref)
      gum_object_ref(obj);
  };
  ~GumObjectWrapper() { gum_object_unref(obj); };

 protected:
  auto get_obj() { return obj; };

 private:
  T* obj;
};
