import React from 'react';
import { Button } from '@mui/material';

export default function JoinButton() {
  return (
    <Button
      sx={{
        height: '40px',
        width: '80px',
        borderRadius: '30px',
        border: '3px solid #9D2933',
        fontSize: 15,
      }}
      variant="outlined"
      color="secondary"
      disabled
    >
      Joined
    </Button>
  );
}
