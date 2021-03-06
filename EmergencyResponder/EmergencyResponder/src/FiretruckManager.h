#ifndef ER_FIRETRUCK_MANAGER_H
#define ER_FIRETRUCK_MANAGER_H

#include "TitanResources.h"

class FiretruckManager
{
public:
	FiretruckManager(std::shared_ptr<titan::api2::ITitan> api);

	/**
	 * Sends the list of firetrucks to the GUI.
	 */
	void SendFiretrucks();
	/**
	 * Saves the location of the mouse position in world space.
	 * @param position A 3D vector with the mouse position
	 */
	void SetMousePosition(titan::api2::Vec3d position);
	/**
	 * Dispatches the firetruck to the current position saved.
	 */
	void DispatchFiretruck();
	/**
	 * Sets the firetruck manager into dispatch mode, in which it will start accepting mouse positions
	 * @param newUuid The uuid of the firetruck currently being dispatched
	 */
	void BeginDispatching(std::string newUuid);
	/**
	 * Returns if the manager is currently in dispatch mode.
	 * @return bool If it is in dispatch mode.
	 */
	bool IsDispatching();

private:
	std::shared_ptr<titan::api2::ITitan> titanApi; /// A reference to the Titan API.
	titan::api2::Vec3d mousePosition; /// The current position of the mouse.
	bool dispatching; /// True if a firetruck is currently being dispatched. False if not.
	std::string uuid; /// The UUID of the firetruck that is currently being dispatched.
	std::map<std::string, std::shared_ptr<titan::api2::IWaypointPath>> m_paths; /// A map of waypoint paths, each firetruck will have their own path.
};

#endif // ER_FIRETRUCK_MANAGER_H