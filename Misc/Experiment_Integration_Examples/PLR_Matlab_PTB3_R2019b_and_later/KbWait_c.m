function KbWait_c(givenKey)
  while 1 % could be stored as a custom function 
    [pressed secs keycode] = KbCheck;
    if pressed
      KbReleaseWait;
      if keycode(givenKey)
        break;
      end
    end
  end
end
