//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2020 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#ifndef TIA_PADDLE_READER
#define TIA_PADDLE_READER

#include "bspf.hxx"
#include "Serializable.hxx"
#include "Console.hxx"

class PaddleReader : public Serializable
{
  public:

    PaddleReader();

  public:

    void reset(uInt64 timestamp);

    void vblank(uInt8 value, uInt64 timestamp);
    bool vblankDumped() const { return myIsDumped; }

    uInt8 inpt(uInt64 timestamp);

    void update(double value, uInt64 timestamp, ConsoleTiming consoleTiming);

    /**
      Serializable methods (see that class for more information).
    */
    bool save(Serializer& out) const override;
    bool load(Serializer& in) override;

  private:

    void setConsoleTiming(ConsoleTiming timing);

    void updateCharge(uInt64 timestamp);

  private:

    double myUThresh{0.0};
    double myU{0.0};

    double myValue{0.0};
    uInt64 myTimestamp{0};

    ConsoleTiming myConsoleTiming;
    double myClockFreq{0.0};

    bool myIsDumped{false};

    static constexpr double
      R0 = 1.5e3,
      C = 68e-9,
      RPOT = 1e6,
      USUPP = 5;

    static constexpr double TRIPPOINT_LINES = 379;

  private:
    PaddleReader(const PaddleReader&) = delete;
    PaddleReader(PaddleReader&&) = delete;
    PaddleReader& operator=(const PaddleReader&) = delete;
    PaddleReader& operator=(PaddleReader&&) = delete;
};

#endif // TIA_PADDLE_READER
