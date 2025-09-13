// ----------------------------------------
// XMFLOAT_HELPER.h
// !!! XMFLOAT 型に四則演算と比較演算子を !!!
// 制作日 2024/11/20 -- arai eito
// 更新日 2024/11/28 -- sakurai yuki
// ----------------------------------------
#ifndef XMFLOAT_HELPER_H_
#define XMFLOAT_HELPER_H_

#include <DirectXMath.h>
#include <initializer_list>
#include <array>
#include <cmath>
#include <cassert>
#include <limits>
/*
template<typename Ty>
_NODISCARD static inline constexpr bool AdjEqual(const Ty epsilon_num, const Ty num)
{
	template <class Ty = double> constexpr Ty _0{ static_cast<Ty>(0.0) };

	static_assert(std::is_floating_point<Ty>(), "This type isn't floating point");

	constexpr auto Fabs{ [](const Ty num) constexpr {
		if(num > _0<Ty>)return num;else return -num;
	}};

	static constexpr auto Epsilon{std::numeric_limits<Ty>::epsilon()};
	auto dis{ Fabs(epsilon_num - num)};

	return (dis <= Epsilon);
}
*/
// ----------------------------------------
// XMFLOAT4
// ----------------------------------------
// 学び inline
// inline キーワードは、コンパイラが関数の各呼び出しの代わりに、
// 関数定義内のコードを書き換えることを示している
// 学び _NODISCARD
// 戻り値の仕様を自動チェックしてくれる属性
// 使用していない場合、コンパイラが警告を出力する
_NODISCARD static inline constexpr auto operator+(
	const DirectX::XMFLOAT4& v1,
	const DirectX::XMFLOAT4& v2)
{
	return DirectX::XMFLOAT4{ v1.x+v2.x , v1.y+v2.y , v1.z+v2.z , v1.w+v2.w };
}
_NODISCARD static inline constexpr auto operator-(
	const DirectX::XMFLOAT4& v1,
	const DirectX::XMFLOAT4& v2)
{
	return DirectX::XMFLOAT4{ v1.x-v2.x , v1.y-v2.y , v1.z-v2.z , v1.w-v2.w };
}
_NODISCARD static inline constexpr auto operator*(
	const DirectX::XMFLOAT4& v,
	const float& f)
{
	return DirectX::XMFLOAT4{ v.x*f , v.y*f , v.z*f , v.w*f };
}
_NODISCARD static inline constexpr auto operator/(
	const DirectX::XMFLOAT4& v,
	const float& f)
{
	return DirectX::XMFLOAT4{ v.x/f , v.y/f , v.z/f , v.w/f };
}
_NODISCARD static inline constexpr auto operator-(
	const DirectX::XMFLOAT4& v)
{
	return DirectX::XMFLOAT4{ -v.x , -v.y , -v.z , -v.w };
}
// ----------------------------------------
// XMFLOAT3 
// ----------------------------------------
_NODISCARD static inline constexpr auto operator+(
	const DirectX::XMFLOAT3& v1,
	const DirectX::XMFLOAT3& v2)
{
	return DirectX::XMFLOAT3{ v1.x + v2.x , v1.y + v2.y , v1.z + v2.z };
}
_NODISCARD static inline constexpr auto operator-(
	const DirectX::XMFLOAT3& v1,
	const DirectX::XMFLOAT3& v2) 
{
	return DirectX::XMFLOAT3{ v1.x - v2.x , v1.y - v2.y , v1.z - v2.z };
}
_NODISCARD static inline constexpr auto operator*(
	const DirectX::XMFLOAT3& v,
	const float& f)
{
	return DirectX::XMFLOAT3{ v.x * f , v.y * f , v.z * f };
}
_NODISCARD static inline constexpr auto operator*(
	const DirectX::XMFLOAT3& v1,
	const DirectX::XMFLOAT3& v2)
{
	return DirectX::XMFLOAT3{ v1.x * v2.x , v1.y * v2.y , v1.z * v2.z };
}
_NODISCARD static inline constexpr auto operator/(
	const DirectX::XMFLOAT3& v,
	const float& f)
{
	return DirectX::XMFLOAT3{ v.x / f , v.y / f , v.z / f };
}

_NODISCARD static inline constexpr auto operator==(
	const DirectX::XMFLOAT3& v,
	const DirectX::XMFLOAT3& f)
{
	return  v.x == f.x && v.y == f.y && v.z == f.z;
}

_NODISCARD static inline constexpr auto operator<=(
	const DirectX::XMFLOAT3& v,
	const DirectX::XMFLOAT3& f)
{
	return  v.x <= f.x && v.y <= f.y && v.z <= f.z;
}

_NODISCARD static inline constexpr auto operator>=(
	const DirectX::XMFLOAT3& v,
	const DirectX::XMFLOAT3& f)
{
	return  v.x >= f.x && v.y >= f.y && v.z >= f.z;
}

// ----------------------------------------
// XMFLOAT2 
// ----------------------------------------
_NODISCARD static inline constexpr auto operator+(
	const DirectX::XMFLOAT2& v1,
	const DirectX::XMFLOAT2& v2)
{
	return DirectX::XMFLOAT2{ v1.x + v2.x , v1.y + v2.y };
}
_NODISCARD static inline constexpr auto operator-(
	const DirectX::XMFLOAT2& v1,
	const DirectX::XMFLOAT2& v2)
{
	return DirectX::XMFLOAT2{ v1.x - v2.x , v1.y - v2.y };
}
_NODISCARD static inline constexpr auto operator*(
	const DirectX::XMFLOAT2& v,
	const float& f)
{
	return DirectX::XMFLOAT2{ v.x * f , v.y * f };
}
_NODISCARD static inline constexpr auto operator/(
	const DirectX::XMFLOAT2& v,
	const float& f)
{
	return DirectX::XMFLOAT2{ v.x / f , v.y / f };
}

#endif