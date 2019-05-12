namespace SongRoll {

  class SongRollData;

  class Transport {
  public:
    Transport(SongRollData* data);
    void reset();

    int currentSection = 0;

  private:
    SongRollData* data;
  };

}
