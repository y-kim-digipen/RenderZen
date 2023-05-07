#pragma once
class IScene
{
public:
	virtual ~IScene() = default;
	virtual bool Init () = 0;
	virtual bool Load () = 0;
	virtual void Update ( float dt ) = 0;
	virtual void Draw () = 0;
	virtual void Unload () = 0;
	virtual void Cleanup () = 0;
};