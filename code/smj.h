class CHello {
 public:
  CHello();
  virtual ~CHello();
  virtual void SayHello();
};

class CHelloA : public CHello {
 public:
  CHelloA();
  ~CHelloA();
  void SayHello();
};
