package com.amplifyreality.networking.model;

import com.amplifyreality.networking.model.*;

public interface RealmPositionWatcher
{
	void PositionUpdate(String objectId, Vector3 newPosition);

	void RotationUpdate(String objectId, Vector3 newRotation);

	void ScaleUpdate(String objectId, Vector3 newScale);
}
