import math
import time
import krpc
import serial

from websocket import create_connection

ser = None
conn = None
ts = int(time.time())

throttlemin = 15
throttlemax = 1010

try:
    ser = serial.Serial('COM3', 38400)
except serial.SerialException:
    print("Is your USB cable connected?")

while conn is None:  # while no connection to kRPC server, try every 15 secs
    try:
        conn = krpc.connect(
            name='KCP',
            address='127.0.0.1',
            rpc_port=50000,
            stream_port=50001)
    except ConnectionRefusedError:
        print("Is KSP running?")
        time.sleep(5)


# vessel setup for first time entering new vessel; TODO - add all arduino switches, leds, get all streams from kRPC
def vessel_setup():  # compare arduino switches with vessel status in game, and adjust switches or vessel as needed to match, arduino toggle switch positions take precedence
    sas = vessel.control.sas  # get current vessel sas status from game;  TODO - get arduino sas switch status, and set vessel sas status to match, set arduino sas mode and wca leds correctly
    rcs = vessel.control.rcs  # get current vessel rcs status from game;  TODO - get arduino rcs switch status, and set vessel sas status to match, set arduino rcs wca led correctly
    target_body('')  # only used for vessels in Kerbin SOI (sphere of influence)


# switch to target vessel
def switch_vessels():
    active_vessel = conn.space_center.active_vessel  # set active_vessel to current vessel
    if conn.space_center.target_vessel is not None:  # if you have another vessel targeted
        target_vessel = conn.space_center.target_vessel  # set target_vessel variable to target
        temp = active_vessel  # set active_vessel to temp
        conn.space_center.active_vessel = target_vessel  # set active_vessel to target
        try:
            target_vessel = temp  # set target_vessel to previous active_vessel, give it time to load
        except:
            time.sleep(0.1)
        vessel_setup()  # run vessel_setup since you switched vessels


# camera modes
def camera(mode):
    if mode == 'automatic':
        conn.space_center.camera.mode = conn.space_center.CameraMode.automatic
    elif mode == 'free':
        conn.space_center.camera.mode = conn.space_center.CameraMode.free
    elif mode == 'chase':
        conn.space_center.camera.mode = conn.space_center.CameraMode.chase
    elif mode == 'locked':
        conn.space_center.camera.mode = conn.space_center.CameraMode.locked
    elif mode == 'orbital':
        conn.space_center.camera.mode = conn.space_center.CameraMode.orbital
    elif mode == 'iva':
        conn.space_center.camera.mode = conn.space_center.CameraMode.iva
    elif mode == 'map':
        conn.space_center.camera.mode = conn.space_center.CameraMode.map
        conn.space_center.navball = True


# target planets and moons
def target_body(celestial_body):
    bodies = conn.space_center.bodies  # get dictionary list for celestial bodies
    body = int(str((bodies[celestial_body]))[
               42:-1])  # input a string to compare to dictionary keys, get the value of that key, convert to string to remove extraneous info, convert number to int
    conn.space_center.target_body = conn.space_center.CelestialBody(body)  # pass the int to the targeting command

    # change autotarget to True if you want vessels to auto-target mun by default
    autotarget = False
    if autotarget is True:
        orbit = vessel.orbit.body.name  # get current closest celestial body
        if orbit == 'Kerbin':  # if vessel is on Kerbin or in Kerbin SOI, and not a space station, set default target to Mun
            if str(vessel.type) != 'VesselType.station':
                try:  # this try is here because some vessels take longer to load, and kRPC would return an error when it couldn't target because the vessel wasn't loaded yet
                    target_body('Mun')
                except:
                    time.sleep(0.1)


# use telemachus to return distance to target object
def distance_to_target():
    ws = create_connection("ws://192.168.1.134:8085/datalink")
    dtt = '{"run":["tar.distance"]}'
    ws.send(dtt)
    data = float(str(ws.recv())[16:-1])
    ws.close()
    return data


# take pitch and heading input from fdcp and turn vessel
def autopilot_pitch_and_heading(pitch, heading):
    sas = vessel.control.sas
    ap = vessel.auto_pilot
    ap.engage()
    ap.target_pitch_and_heading(pitch, heading)
    ap.wait()
    if sas is True:
        vessel.control.sas = True
    ap.disengage()


# used for vector math for finding ejection angle
def cross(u, v):
    return (
        u[1] * v[2] - u[2] * v[1],
        u[2] * v[0] - u[0] * v[2],
        u[0] * v[1] - u[1] * v[0])


# used for vector math for finding ejection angle
def dot(u, v):
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2]


# calculate transfer window
def planetary_transfer(target_planet):
    sas = vessel.control.sas
    mu = conn.space_center.CelestialBody(
        int(str((conn.space_center.bodies['Sun']))[42:-1])).gravitational_parameter  # gravitational constant of kerbol
    r = conn.space_center.CelestialBody(int(str((conn.space_center.bodies[target_planet]))[
                                            42:-1])).orbit.semi_major_axis  # semi-major axis of the target planet (a = (r1 + r2)/2) r1 is apoapsis and r2 is periapsis)
    k = conn.space_center.CelestialBody(
        int(str(vessel.orbit.body)[42:-1])).orbit.semi_major_axis  # semi-major axis of the current planet
    a = (
                k + r) / 2.  # a = (r1 + r2)/2   r1 is semi-major axis of current planet and r2 is semi-major axis of target planet
    t = (2. * math.pi * math.sqrt(
        a ** 3 / mu)) / 2.  # Kepler's Third Law (2*PI*sqrt((a^3)/GM)) divide this by 2 since we are only traveling one way; https://en.wikipedia.org/wiki/Kepler%27s_laws_of_planetary_motion#Third_law_of_Kepler
    phase_angle = 180 - math.sqrt(
        mu / r) * t / r * 180 / math.pi  # optimal angle between current planet and target planet for maneuver node

    soi = conn.space_center.CelestialBody(
        int(str(vessel.orbit.body)[42:-1])).sphere_of_influence  # SOI of current planet
    d = vessel.orbit.radius  # radius of vessel from center of mass
    v = math.sqrt(mu / (soi + k)) * (
            math.sqrt((2. * r) / (soi + k + r)) - 1.)  # change in velocity needed for the Hohmann transfer
    kmu = conn.space_center.CelestialBody(
        int(str(vessel.orbit.body)[42:-1])).gravitational_parameter  # gravitational constant of current planet
    ejection_velocity = math.sqrt((d * (soi * v ** 2 - 2. * kmu) + 2. * soi * kmu) / (
            d * soi))  # velocity required to escape current planet SOI and rendezvous with target planet

    ejection_angle = 180 - math.degrees(math.acos(math.sqrt(kmu ** 2 / (
            kmu - ejection_velocity ** 2 * d) ** 2)))  # angle of current orbit between vessel and planet prograde/retrograde path

    dtt = distance_to_target()  # distance from origin planet to target
    ct = conn.space_center.CelestialBody(int(str((conn.space_center.bodies[target_planet]))[
                                             42:-1])).orbit.radius  # current target's distance from kerbol
    co = conn.space_center.CelestialBody(
        int(str(vessel.orbit.body)[42:-1])).orbit.radius  # origin planet's distance from kerbol
    current_angle = 180 - (math.degrees(math.cos((ct ** 2 + co ** 2 - dtt ** 2) / (
            2 * ct * co))))  # current phase angle between origin and target planet; cos(C) = (a^2 + b^2 - c^2)/(2*a*b)
    ut = conn.add_stream(getattr, conn.space_center, 'ut')  # universal time
    cop = conn.space_center.CelestialBody(
        int(str(vessel.orbit.body)[42:-1])).orbit.period  # origin planet's orbital period
    top = conn.space_center.CelestialBody(
        int(str((conn.space_center.bodies[target_planet]))[42:-1])).orbit.period  # target planet's orbital period
    cdc = cop / 360  # how many seconds origin planet takes to move 1 degree
    tdc = cdc / top * 360  # how many degrees does target planet move in relation to origin planet

    angle_dif = current_angle - phase_angle
    time_to_phase_angle = ((
                                   angle_dif / tdc) / 360) * cop  # how many seconds until current angle equals optimal phase angle
    delta_v = (ejection_velocity - math.sqrt(kmu / d))  # delta_v required for ejection burn to reach ejection velocity

    time_to_ejection_angle = 0  # TODO - figure out current angle in relation to origin planet's prograde movement

    # these vectors are all in orbital_reference_frame
    prograde = (0, 1, 0)
    retrograde = (0, -1, 0)
    radial_out = (-1, 0, 0)
    radial_in = (1, 0, 0)
    normal = (0, 0, 1)
    antinormal = (0, 0, -1)

    reference_frame = conn.space_center.CelestialBody(int(str(vessel.orbit.body)[42:-1])).orbital_reference_frame

    # TODO - figure out final node position and place it in relation to ejection angle

    # plane_outer = cross()

    # angle between plane
    # print(math.degrees(math.asin(dot(plane_outer, prograde)) * (180.0 / math.pi)))

    # calculate burn time (using Tsiolkovsky rocket equation: https://en.wikipedia.org/wiki/Tsiolkovsky_rocket_equation)
    F = vessel.available_thrust
    Isp = vessel.specific_impulse * 9.82
    m0 = vessel.mass
    m1 = m0 / math.exp(delta_v / Isp)
    flow_rate = F / Isp
    burn_time = (m0 - m1) / flow_rate

    node = vessel.control.add_node(ut() + time_to_phase_angle + time_to_ejection_angle, prograde=delta_v,
                                   radial=0)  # create maneuver node

    ap = vessel.auto_pilot
    ap.engage()
    ap.reference_frame = node.reference_frame
    ap.target_direction = (0, 1, 0)
    ap.wait()

    burn_ut = ut() + time_to_phase_angle + time_to_ejection_angle - (burn_time / 2.)
    lead_time = 10
    conn.space_center.warp_to(burn_ut - lead_time)

    time_to_node = node.time_to
    while time_to_node - (burn_time / 2.) > 0:
        pass
    time.sleep(burn_time - 0.0)
    vessel.control.throttle = 0.05
    remaining_burn = conn.add_stream(node.remaining_burn_vector, node.reference_frame)
    while remaining_burn()[1] > 3.0:
        pass
    vessel.control.throttle = 0.0
    node.remove()
    if sas is True:
        vessel.control.sas = True
    ap.disengage()


# circularize orbit at current apoapsis
def circularize_orbit():
    ap = vessel.auto_pilot
    sas = vessel.control.sas
    ut = conn.add_stream(getattr, conn.space_center, 'ut')

    # calculate delta-v required to circularize the orbit (using vis-viva equation: https://en.wikipedia.org/wiki/Vis-viva_equation)
    mu = vessel.orbit.body.gravitational_parameter
    r = vessel.orbit.apoapsis
    a1 = vessel.orbit.semi_major_axis
    a2 = r
    v1 = math.sqrt(mu * ((2. / r) - (1. / a1)))
    v2 = math.sqrt(mu * ((2. / r) - (1. / a2)))
    delta_v = v2 - v1
    node = vessel.control.add_node(ut() + vessel.orbit.time_to_apoapsis, prograde=delta_v)

    # calculate burn time (using Tsiolkovsky rocket equation: https://en.wikipedia.org/wiki/Tsiolkovsky_rocket_equation)
    F = vessel.available_thrust
    Isp = vessel.specific_impulse * 9.82
    m0 = vessel.mass
    m1 = m0 / math.exp(delta_v / Isp)
    flow_rate = F / Isp
    burn_time = (m0 - m1) / flow_rate

    ap.engage()
    ap.reference_frame = node.reference_frame
    ap.target_direction = (0, 1, 0)
    ap.wait()

    burn_ut = ut() + vessel.orbit.time_to_apoapsis - (burn_time / 2.)
    lead_time = 5
    conn.space_center.warp_to(burn_ut - lead_time)

    time_to_apoapsis = conn.add_stream(getattr, vessel.orbit, 'time_to_apoapsis')
    while time_to_apoapsis() - (burn_time / 2.) > 0:
        pass
    vessel.control.throttle = 1.0
    time.sleep(burn_time - 0.0)
    vessel.control.throttle = 0.05
    remaining_burn = conn.add_stream(node.remaining_burn_vector, node.reference_frame)
    while remaining_burn()[1] > 3.0:
        pass
    vessel.control.throttle = 0.0
    node.remove()
    if sas is True:
        vessel.control.sas = True
    ap.disengage()


# node to target
def node_to_target():
    ap = vessel.auto_pilot


# takes input as string
# returns true if input is "good" per protocol
def checkinput(i):
    for c in i:
        c = ord(c)
        if c >= 48 and c <= 57:  # allow numbers
            continue

        elif c == 33 or c == 46:  # allow ! and .
            continue
        else:
            print("x" * 10)
            print("GO CHECK THIS", i)
            print("x" * 10)
            return False
    return True


# accept an integer throttle input between 15 and 1010 and create a return such that 0<=ret<=1
def throttlecontrol(throttleinput):
    # set the return function to the equation of the line between min and max
    # we use the equation generated by wolfram alpha
    ret = round((throttleinput - throttlemin) / (throttlemax - throttlemin), 3)
    if ret < 0.1:
        return 0.0
    return ret


# take a str,int, bool or float and craft a <=12 character byte array encoded with latin-1
# truncates if input is too long.
# returns the number of bytes sent
def output(out):
    size = 12
    if type(out) == str:
        pass
    if type(out) == int:
        out = str(out)[:size]
    elif type(out) == bool:
        if out:
            out = 1
        else:
            out = 0
        out = str(out)
    elif type(out) == float:
        out = str(int(round(out, 0)))[:size]
    out = ((size - len(out)) * "0") + out

    return ser.write(out.encode('latin-1'))


# takes an integer value between 1- 1024 and generates a float between -1,1
def fcs(input):
    fc = round((int(input) / 512) - 1, 2)
    return fc


# boolean for testing sync
sync = True

vessel = None
setup = False
check = False

while True:  # main loop
    if conn is not None:  # if connected to kRPC
        while vessel is None:  # while not in a vessel
            try:  # try to connect to vessel every 5 seconds
                vessel = conn.space_center.active_vessel
            except:  # ignore KSP errors that you're not in a vessel
                time.sleep(5)

        while True:  # main vessel loop
            if setup is False:  # if vessel setup is not True
                try:
                    vessel_setup()  # run vessel setup
                except:  # ignore KSP errors while you're loading into the vessel
                    time.sleep(0.1)
                setup = True  # only allow setup to run when first entering a vessel

            cts = int(time.time())  # current time stamp
            if int((cts - ts) % 10) == 0 and check is False:  # every 10 seconds
                for i in range(0, 1):  # check 1 time
                    try:  # verify still in vessel
                        vessel = conn.space_center.active_vessel
                        check = True  # mark vessel as checked
                    except:  # ignore KSP errors that you're not in a vessel
                        vessel = None  # clear vessel
                        setup = False  # reset vessel setup to false
            elif int((cts - ts) % 10) == 1:  # reset vessel check to false
                check = False

            if vessel is None:
                break  # break out of main vessel loop to connect to new vessel

            # try to get 16 characters this try block is crap
            try:
                i = ser.read(16).decode("latin-1")
            except ValueError as e:
                print(e, i)
                # convert the bytes to a string
            i = str(i)
            if i[3] == "." or i[0] == "!":
                sync = True
            else:
                print("p sync is BAD")
                sync = False

            while sync is not True:
                try:
                    i = ser.read(1).decode("latin-1")
                except ValueError as e:
                    print(e, i)
                try:
                    i = ser.read(16).decode("latin-1")
                except ValueError as e:
                    print(e, i)
                i = str(i)
                if input[0] == "!" or input[3] == ".":
                    print("p sync is good")
                    sync = True

            if i[0] == "!":
                # example string looks like !00234.1000.0001
                # remove the ! and split the data into a list
                data = i[1:].split(".")
                # for example data would now contain [00234,1000,1]
                yaw, pitch, roll = data
                # yaw, pitch and roll take those three values and are converted
                vessel.control.yaw = fcs(yaw)
                # good link to the API
                # https://krpc.github.io/krpc/python/api/space-center/control.html#SpaceCenter.Control.pitch
                vessel.control.pitch = fcs(pitch)
                vessel.control.roll = fcs(roll)
                continue
            elif "500" in i:
                # same premise except the "command" sent is 500. This is throttle control
                # same thought process as above.
                # we floor and max values at the min and max throttles to send allowed by arduino
                # to get the vessel to behave as expected
                data = i.split(".")
                throttle = int(data[1])
                if throttle < throttlemin:
                    throttle = throttlemin
                if throttle > throttlemax:
                    throttle = throttlemax
                throttle = throttlecontrol(throttle)
                vessel.control.throttle = throttle
                continue
            elif "501" in i:
                # throttle setup used at the beginning of sync with krpc and arduino
                # essentially, read the throttle of the vessel in game
                throttle = vessel.control.throttle
                # this is the reverse of the equation in throttlecontrol()
                out = (throttlemax - throttlemin) * throttle + throttlemin
                out = int(out)
                # floor it, we cannot set the throttle to a float
                output(out)
                continue
            elif "601" in i:
                altitude = vessel.flight().mean_altitude
                output(altitude)
                continue
            elif "602" in i:
                apoapsis = vessel.orbit.apoapsis_altitude
                output(apoapsis)
                continue
            elif "603" in i:
                periapsis = vessel.orbit.periapsis_altitude
                output(periapsis)
                continue
            elif "604" in i:
                orbitspeed = vessel.orbit.speed
                output(orbitspeed)
                continue

            elif "605" in i:
                for j in range(10, 0, -1):
                    resources = vessel.resources_in_decouple_stage(stage=j, cumulative=False)
                    liquidfuel_amount = resources.amount('LiquidFuel')
                    liquidfuel_max = resources.max('LiquidFuel')
                    if liquidfuel_amount != 0:
                        liquidfuel_pct = round(((liquidfuel_amount / liquidfuel_max) * 100))
                        output(liquidfuel_pct)
                        break
                continue

            elif "606" in i:
                for j in range(10, 0, -1):
                    resources = vessel.resources_in_decouple_stage(stage=j, cumulative=False)
                    oxidizer_amount = resources.amount('Oxidizer')
                    oxidizer_max = resources.max('Oxidizer')
                    if oxidizer_amount != 0:
                        oxidizer_pct = round(((oxidizer_amount / oxidizer_max) * 100))
                        ox = ((200 - (oxidizer_pct * 2)) + 9)
                        output(ox)
                        break
                continue

            elif "608" in i:
                for j in range(10, 0, -1):
                    resources = vessel.resources_in_decouple_stage(stage=j, cumulative=False)
                    oxidizer_max = resources.max('Oxidizer')
                    if oxidizer_max != 0:
                        output(oxidizer_max)
                        break
                    continue
                continue

            elif "609" in i:
                resources = vessel.resources
                xenon = resources.amount('Xenon')
                output(xenon)
                continue

            elif "611" in i:
                resources = vessel.resources
                solidfuel = resources.amount('SolidFuel')
                output(solidfuel)
                continue

            elif "999" in i:
                print("p", i)

            elif "301" in i:  # execute button pressed
                vessel.control.activate_next_stage()
                continue
            elif "302" in i:  # abort button pressed
                vessel.control.abort = True
                continue

            elif "400" in i:
                vessel.control.sas = True
                continue

            elif "401" in i:
                vessel.control.sas_mode = conn.space_center.SASMode.stability_assist
                continue

            elif "402" in i:
                # @TODO ADD CATCH
                try:
                    vessel.control.sas_mode = conn.space_center.SASMode.maneuver
                except RuntimeError:
                    print("p maneuver not available")
                continue

            elif "403" in i:
                try:
                    vessel.control.sas_mode = conn.space_center.SASMode.prograde
                except RuntimeError:
                    print("p PROGRADE not available")
                continue

            elif "404" in i:
                try:
                    vessel.control.sas_mode = conn.space_center.SASMode.retrograde
                except RuntimeError:
                    print("p RETROGRADE not available")
                continue

            elif "405" in i:
                vessel.control.sas_mode = conn.space_center.SASMode.normal
                continue

            elif "406" in i:
                vessel.control.sas_mode = conn.space_center.SASMode.anti_normal
                continue

            elif "407" in i:
                vessel.control.sas_mode = conn.space_center.SASMode.radial
                continue

            elif "408" in i:
                vessel.control.sas_mode = conn.space_center.SASMode.anti_radial
                continue

            elif "409" in i:
                # @TODO ADD CATCH
                try:
                    vessel.control.sas_mode = conn.space_center.SASMode.target
                except RuntimeError:
                    print("p target not available")
                continue

            elif "410" in i:
                # @TODO ADD CATCH
                try:
                    vessel.control.sas_mode = conn.space_center.SASMode.anti_target
                except RuntimeError:
                    print("p anti target not available")
                continue

            elif "411" in i:
                vessel.control.sas = False
                continue
