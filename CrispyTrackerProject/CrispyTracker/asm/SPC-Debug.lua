SineName = "pos_1_5"
PCVal = 0x05FD
SinArr = {}
SinInd = 0
function PrintSineValue()
	STab = emu.getState()
	if STab["spc.pc"] == PCVal then
		YVal = STab["spc.y"]
		if YVal >= 128 then
			YVal = YVal - 256
		end
		SinArr[SinInd] = YVal
		SinInd = (SinInd + 1)%256
		SinArr[SinInd] = SinArr[SinInd]
		emu.log(tostring(YVal))
		--emu.resume()
	end
	--STab.spc.y
end

function DrawSine()
	for a = 0, #SinArr do
		emu.drawLine(a, SinArr[a]+128, a+1, SinArr[a+1]+128)
	end
end

--Register some code (printInfo function) that will be run at the end of each frame
emu.getLabelAddress(SineName)
emu.addEventCallback(PrintSineValue, emu.eventType.codeBreak)
emu.addEventCallback(DrawSine, emu.eventType.startFrame)