class Datum {
  private:    
  public:
    struct t_date {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        };  
    Datum();
    t_date getDate(double);
    int getHour(String, char);
    int getMinute(String, char);
    int getHours();
    int getMinutes();
    int getDay();
    void getDatum();
    void update();
    String getFormattedTime();
    t_date initTimeServer(t_date);
    t_date updateTimeServer(t_date);
};
