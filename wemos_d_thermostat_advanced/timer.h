class Timer {
  private:    
    #define AUTORESET true
    #define NOAUTORESET false
    unsigned long startTime;
    unsigned long waitTime;
  public:  
    Timer(int ms, bool autoreset);
    void reset (int ms = 0);
    bool timeout ();
    void flush ();
    int remaining ();
    int get ();
    bool autoReset;
};