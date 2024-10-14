--[[
    Last edited 13.10.2024 by jxded
    App is provided as is, no liabilities!
    Please share with credits
    github.com/jxded www.youtube.com/@zaks7
    thank you to @JanTrueno
--]]


-- user vars
local trailDuration = 2.3 
local fadeDuration = 0.3  
local stickRange = 80   
local newsPosX, newsPosY = 520, 135
local buttonOffset = 35
local dpadPosX, dpadPosY = 80 , 75
local arrowOffset = 10
-- internal vars
local gamepads = {}  
local gamepadIndex = 1 -- Multiple gamepads (test) [13.10.2024]
local gamepad
local buttonStates = {}
local dpadStates = {
    dpup = false,
    dpdown = false,
    dpleft = false,
    dpright = false
}
local arrowAngle = 0
local leftStickTrail = {}
local rightStickTrail = {}
local leftXRaw, leftYRaw, rightXRaw, rightYRaw = 0, 0, 0, 0

function love.load()
    love.graphics.setBackgroundColor(0, 0, 0)
    love.window.setMode(640, 480) 

    --debug line, run from console to get mapping string for your device if you need!
    -- local mappingstring = gamepad:getGamepadMappingString( )
    --     print(mappingstring)

    --[[
    gamepads = love.joystick.getJoysticks()
    print(#gamepads)
    
    if #gamepads > 0 then
        gamepad = gamepads[1]
    end
    --]]
end

-- Add gamepad detection
function love.joystickadded(joystick)
    
    if joystick:isGamepad() then
        table.insert(gamepads, joystick)  -- Add new gamepad to the list
        if not gamepad then
            gamepad = gamepads[gamepadIndex]  -- Set current gamepad if none selected
        end
    end
    print("Joystick added: " .. #gamepads)
end

function love.joystickremoved(joystick)
    for i, gp in ipairs(gamepads) do
        if gp == joystick then
            table.remove(gamepads, i) 
            break
        end
    end
    print("Joystick removed: " .. #gamepads)

    -- remove current gamepad reference if it's removed
    if gamepad == joystick then
        if #gamepads > 0 then
            gamepadIndex = 1 
            gamepad = gamepads[gamepadIndex]  -- defaults to inbuilt controller
        else
            gamepad = nil
        end
    end
end

function cycleGamepads()
    if #gamepads > 1 then
        gamepadIndex = (gamepadIndex % #gamepads) + 1  
        gamepad = gamepads[gamepadIndex]  
    end
end


function love.update(dt)
    if gamepad then
        -- Store raw stick positions
        leftXRaw = gamepad:getGamepadAxis("leftx")  -- left stick x-axis
        leftYRaw = gamepad:getGamepadAxis("lefty")  -- left stick y-axis
        rightXRaw = gamepad:getGamepadAxis("rightx") -- right stick x-axis
        rightYRaw = gamepad:getGamepadAxis("righty") -- right stick y-axis


        table.insert(leftStickTrail, {x = leftXRaw, y = leftYRaw, time = love.timer.getTime()})
        table.insert(rightStickTrail, {x = rightXRaw, y = rightYRaw, time = love.timer.getTime()})

        
        -- Handle trail points that have passed their lifespam
        removeOldTrailPoints(leftStickTrail)
        removeOldTrailPoints(rightStickTrail)

        buttonStates["a"] = gamepad:isGamepadDown("a")
        buttonStates["b"] = gamepad:isGamepadDown("b")
        buttonStates["x"] = gamepad:isGamepadDown("x")
        buttonStates["y"] = gamepad:isGamepadDown("y")
        buttonStates["select"] = gamepad:isGamepadDown("back")
        buttonStates["start"] = gamepad:isGamepadDown("start")
        buttonStates["function"] = gamepad:isGamepadDown("guide")
        buttonStates["ls"] = gamepad:isGamepadDown("leftshoulder")
        buttonStates["lt"] = gamepad:getGamepadAxis("triggerleft")
        buttonStates["rs"] = gamepad:isGamepadDown("rightshoulder")
        buttonStates["rt"] = gamepad:getGamepadAxis("triggerright")
        buttonStates["l3"] = gamepad:isGamepadDown("leftstick")
        buttonStates["r3"] = gamepad:isGamepadDown("rightstick")

        dpadStates["dpup"] = gamepad:isGamepadDown("dpup")
        dpadStates["dpdown"] = gamepad:isGamepadDown("dpdown")
        dpadStates["dpleft"] = gamepad:isGamepadDown("dpleft")
        dpadStates["dpright"] = gamepad:isGamepadDown("dpright")

        -- Check for quit command (Select + start)
    if gamepad:isGamepadDown("back") and gamepad:isGamepadDown("start") then
        love.event.quit() 
        return
    end
    -- Check for cycle gamepad command (LB + RB) [Test]
    if gamepad:isGamepadDown("leftshoulder") and gamepad:isGamepadDown("rightshoulder") then
        cycleGamepads()
    end
    end
end


function removeOldTrailPoints(trail)
    local currentTime = love.timer.getTime()
    for i = #trail, 1, -1 do
        if currentTime - trail[i].time > trailDuration + fadeDuration then
            table.remove(trail, i)
        end
    end
end

function love.draw()
    if gamepad then

        drawStickTrail(leftStickTrail, 160, 320, {0, 1, 0}) 
        drawStickTrail(rightStickTrail, 480, 320, {1, 0, 0}) 

        -- left stick outline and position line
        drawStickOutline(160, 320, leftStickTrail, {0.7, 0.7, 0.7, 0.5}, "l3") 
        drawStickOutline(480, 320, rightStickTrail, {0.7, 0.7, 0.7, 0.5}, "r3") 

        drawButtons()
        drawDpad()

        -- raw analog stick values at the bottom
        love.graphics.setColor(1, 1, 1) 
        love.graphics.print(string.format("(%.2f, %.2f)", leftXRaw, leftYRaw), 125, 410)
        love.graphics.print(string.format("(%.2f, %.2f)", rightXRaw, rightYRaw), 445, 410)

        -- quit instruction
        love.graphics.setColor(1, 1, 1)
        love.graphics.print("Press Select + Start to quit", 250, 50)
        
        -- gamepad # hints
        love.graphics.print("Connected: " .. gamepad:getName(), 20, 440)
        if #gamepads > 1 then
            love.graphics.print("Press LB + RB to cycle gamepads", 20, 460)
        end
        -- creds, dont erase please! 
        love.graphics.print("¦¦ github/jxded ¦¦ youtube/@zaks7 ¦¦", 410, 460)
    else
        love.graphics.print("No gamepad detected", 250, 240)
        love.graphics.print("¦¦ github/jxded ¦¦ youtube/@zaks7 ¦¦", 410, 460)
    end
end

-- analog stick trails
function drawStickTrail(trail, centerX, centerY, color)
    local currentTime = love.timer.getTime()
    for _, point in ipairs(trail) do
        local timeSince = currentTime - point.time
        if timeSince > trailDuration then
            local fadeAmount = 1 - (timeSince - trailDuration) / fadeDuration
            love.graphics.setColor(color[1], color[2], color[3], fadeAmount)
        else
            love.graphics.setColor(color[1], color[2], color[3], 1)
        end
        love.graphics.circle("fill", centerX + point.x * stickRange, centerY + point.y * stickRange, 5)
    end
end

-- analog stick position
function drawStickOutline(centerX, centerY, trail, lineColor, which)

    love.graphics.setColor(1, 1, 1, 0.2) 
    love.graphics.circle("line", centerX, centerY, stickRange)

    if #trail > 0 then
        local currentPoint = trail[#trail]
        local stickX = centerX + currentPoint.x * stickRange
        local stickY = centerY + currentPoint.y * stickRange
        
        love.graphics.setColor(lineColor)
        love.graphics.line(centerX, centerY, stickX, stickY)

        local dotRadius
        if buttonStates[which] then
            love.graphics.setColor(1, 1, 1, 1)
            dotRadius = 15
        else
            love.graphics.setColor(0.2, 0.2, 0.2) 
            dotRadius = 7
        end
        love.graphics.circle("fill", stickX, stickY, dotRadius)
    end
end

-- Button feedback
function drawButtons()
    -- Scaled button positions for 640x480 [11.10.2024]

    -- Button A
    local typeA = ""
    if buttonStates["a"] then
        love.graphics.setColor(1, 0, 0) 
        typeA = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeA = "line"
    end

    love.graphics.circle(typeA, newsPosX + buttonOffset, newsPosY, 15)


    -- Button B
    local typeB = ""
    if buttonStates["b"] then
        love.graphics.setColor(1, 1, 0) 
        typeB = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeB = "line"
    end

    love.graphics.circle(typeB, newsPosX, newsPosY + buttonOffset, 15)
    

    -- button X
    local typeX = ""
    if buttonStates["x"] then
        love.graphics.setColor(0, 0, 1) 
        typeX = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeX = "line"
    end

    love.graphics.circle(typeX, newsPosX, newsPosY - buttonOffset, 15)

    -- button Y
    local typeY = ""
    if buttonStates["y"] then
        love.graphics.setColor(0, 1, 0) 
        typeY = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeY = "line"
    end

    love.graphics.circle(typeY, newsPosX - buttonOffset, newsPosY, 15)

    -- button Select
    local typeSelect = ""
    if buttonStates["select"] then
        love.graphics.setColor(0.3, 0.3, 0.3) 
        typeSelect = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeSelect = "line"
    end

    love.graphics.circle(typeSelect, 240, 180, 15)

    -- button Start
    local typeStart = ""
    if buttonStates["start"] then
        love.graphics.setColor(0.3, 0.3, 0.3) 
        typeStart = "fill" 
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeStart = "line"
    end

    love.graphics.circle(typeStart, 400, 180, 15)

    -- button Function
    local typeF = ""
    if buttonStates["function"] then
        love.graphics.setColor(0.3, 0.3, 0.3) 
        typeF = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeF = "line"
    end

    love.graphics.circle(typeF, 320, 120, 15)

    -- button Left Shoulder
    local typeLS = ""
    if buttonStates["ls"] then
        love.graphics.setColor(0.5, 0.5, 0.5) 
        typeLS = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeLS = "line"
    end

    love.graphics.rectangle(typeLS, 40, 60, 30, 30 )

    -- button Left Shoulder
    local typeRS = ""
    if buttonStates["rs"] then
        love.graphics.setColor(0.5, 0.5, 0.5) 
        typeRS = "fill"
    else
        love.graphics.setColor(0.2, 0.2, 0.2) 
        typeRS = "line"
    end

    love.graphics.rectangle(typeRS, 570, 60, 30, 30 )

    -- axis Left Trigger
    love.graphics.setColor(0.2, 0.2, 0.2) 
    love.graphics.rectangle("line", 120, 20, 60, 30 )
    love.graphics.setColor(1,1,1,buttonStates["lt"])
    love.graphics.rectangle("fill", 120, 20, 60, 30 )

    -- axis Right Trigger
    love.graphics.setColor(0.2, 0.2, 0.2) 
    love.graphics.rectangle("line", 460, 20, 60, 30 )
    love.graphics.setColor(1,1,1,buttonStates["rt"])
    love.graphics.rectangle("fill", 460, 20, 60, 30 )

end

function drawDpad()
    local dpadSpriteLoad
    local arrowSprite = love.graphics.newImage("assets/arrow2.png")
    -- Check if no D-pad buttons are pressed
    local allReleased = true
    for _, state in pairs(dpadStates) do
        if state then
            allReleased = false
            break
        end
    end

    -- handdrawn sprites, sorry
    if allReleased then
        dpadSpriteLoad = "assets/dpad_empty.png"
    elseif dpadStates["dpup"] and dpadStates["dpleft"] then -- Diagonal up-left
        dpadSpriteLoad = "assets/dpad_diag_lu.png"  
        arrowAngle = math.rad(-135)
    elseif dpadStates["dpup"] and dpadStates["dpright"] then -- Diagonal up-right
        dpadSpriteLoad = "assets/dpad_diag_ru.png"  
        arrowAngle = math.rad(-45)
    elseif dpadStates["dpdown"] and dpadStates["dpleft"] then -- Diagonal down-left
        dpadSpriteLoad = "assets/dpad_diag_ld.png" 
        arrowAngle = math.rad(135)
    elseif dpadStates["dpdown"] and dpadStates["dpright"] then -- Diagonal down-right
        dpadSpriteLoad = "assets/dpad_diag_rd.png"  
        arrowAngle = math.rad(45)
    elseif dpadStates["dpup"] then
        dpadSpriteLoad = "assets/dpad_up.png"
        arrowAngle = math.rad(-90)
    elseif dpadStates["dpdown"] then
        dpadSpriteLoad = "assets/dpad_down.png"
        arrowAngle = math.rad(90)
    elseif dpadStates["dpleft"] then
        dpadSpriteLoad = "assets/dpad_left.png"
        arrowAngle = math.rad(180)
    elseif dpadStates["dpright"] then
        dpadSpriteLoad = "assets/dpad_right.png"
        arrowAngle = math.rad(0)
    end

    if dpadSpriteLoad then
        local dpadSpriteDraw = love.graphics.newImage(dpadSpriteLoad)
        love.graphics.setColor(0.8, 0.8, 0.8) 
        love.graphics.draw(dpadSpriteDraw, dpadPosX, dpadPosY)
    end
    
    if not allReleased then
        local arrowX = dpadPosX + 60 + math.cos(arrowAngle) * arrowOffset
        local arrowY = dpadPosY + 60 + math.sin(arrowAngle) * arrowOffset
        love.graphics.draw(arrowSprite, arrowX, arrowY, arrowAngle, 1, 1, - 60, arrowSprite:getHeight()/2)
    end
end


