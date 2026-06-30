%module(directors="1") rust_director_create_callback

%feature("director") MessageReceiver;

%inline %{

class MessageReceiver {
public:
  MessageReceiver() {
  }
  virtual ~MessageReceiver() {
  }
  virtual int receive(int message) {
    return message;
  }
};

class MessageBus {
public:
  MessageBus() : receiver(0) {
  }
  void create(MessageReceiver *callback) {
    receiver = callback;
  }
  int dispatch(int message) {
    return receiver ? receiver->receive(message) : -1;
  }
  void clear() {
    receiver = 0;
  }
private:
  MessageReceiver *receiver;
};

%}
