function KbWait_c_esc(givenKey)
  while 1 % could be stored as a custom function 
    [pressed secs keycode] = KbCheck;
    if pressed
      KbReleaseWait;
      if keycode(27)
        error('Terminated by user');
      end
      if keycode(givenKey)
        break;
      end
    end
  end
end
