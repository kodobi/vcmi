#ifndef INT3_H
#define INT3_H
#include <map>
class CCreature;
class CCreatureSet //seven combined creatures
{
public:
	std::map<si32,std::pair<CCreature*,si32> > slots;
	bool formation; //false - wide, true - tight
};

class int3
{
public:
	si32 x,y,z;
	inline int3():x(0),y(0),z(0){}; //c-tor, x/y/z initialized to 0
	inline int3(const si32 & X, const si32 & Y, const si32 & Z):x(X),y(Y),z(Z){}; //c-tor
	inline ~int3(){} // d-tor - does nothing
	inline int3 operator+(const int3 & i) const
		{return int3(x+i.x,y+i.y,z+i.z);}
	inline int3 operator+(const si32 i) const //increases all components by si32
		{return int3(x+i,y+i,z+i);}
	inline int3 operator-(const int3 & i) const
		{return int3(x-i.x,y-i.y,z-i.z);}
	inline int3 operator-(const si32 i) const
		{return int3(x-i,y-i,z-i);}
	inline int3 operator-() const //increases all components by si32
		{return int3(-x,-y,-z);}
	inline void operator+=(const int3 & i)
	{
		x+=i.x;
		y+=i.y;
		z+=i.z;
	}	
	inline void operator+=(const si32 & i)
	{
		x+=i;
		y+=i;
		z+=i;
	}
	inline void operator-=(const int3 & i)
	{
		x-=i.x;
		y-=i.y;
		z-=i.z;
	}	
	inline void operator-=(const si32 & i)
	{
		x+=i;
		y+=i;
		z+=i;
	}	
	inline bool operator==(const int3 & i) const
		{return (x==i.x) && (y==i.y) && (z==i.z);}	
	inline bool operator!=(const int3 & i) const
		{return !(*this==i);}
	inline bool operator<(const int3 & i) const
	{
		if (z<i.z)
			return true;
		if (z>i.z)
			return false;
		if (y<i.y)
			return true;
		if (y>i.y)
			return false;
		if (x<i.x)
			return true;
		if (x>i.x)
			return false;
		return false;
	}
	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & x & y & z;
	}
};
inline std::istream & operator>>(std::istream & str, int3 & dest)
{
	str>>dest.x>>dest.y>>dest.z;
	return str;
}
inline std::ostream & operator<<(std::ostream & str, const int3 & sth)
{
	return str<<sth.x<<' '<<sth.y<<' '<<sth.z;
}
#endif //INT3_H