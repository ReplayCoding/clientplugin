#include <KeyValues.h>
#include <convar.h>

#include "modules/modules.hpp"
#include "offsets/offsets.hpp"

struct CEconNotification {
  virtual ~CEconNotification();

  // lol
  virtual void SetLifetime(float flSeconds);

  const char* text;
  const char* soundname;
};

typedef void* (*CEconNotification_constructor)(CEconNotification* this_);
typedef void* (*CEconNotification_setkv_t)(CEconNotification* this_,
                                           KeyValues* kv);
typedef void* (*NotifQueue_Add_Func)(CEconNotification* notif);

class NotificationMod : public IModule {
 public:
  NotificationMod();

 private:
  static void show_notification();
  ConCommand show_notification_cmd;
};

NotificationMod::NotificationMod()
    : show_notification_cmd("fh_show_notification", show_notification) {}

void NotificationMod::show_notification() {
  CEconNotification* notif = (CEconNotification*)malloc(0x102c);

  auto* cons = static_cast<CEconNotification_constructor>(
      offsets::CEconNotification_CEconNotification);
  cons(notif);

  notif->text = "FIENDHOOK MESSAGE: ducks";
  notif->soundname = "ambient/bumper_car_quack2.wav";
  notif->SetLifetime(90.f);

  // Color color(0xff, 0xcc, 0x33, 255);
  // KeyValuesAD KV("System Message");
  // KV->SetWString("message", L"wow");
  // KV->SetColor("custom_color", color);

  // auto* setkv = static_cast<CEconNotification_setkv_t>(
  //     offsets::CEconNotification_SetKeyValues);
  // setkv(notif, KV);

  auto* notify =
      static_cast<NotifQueue_Add_Func>(offsets::NotificationQueue_Add);
  notify(notif);
}

REGISTER_MODULE(NotificationMod)
