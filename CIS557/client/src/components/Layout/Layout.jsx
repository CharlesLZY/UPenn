import React, { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { styled, alpha } from '@mui/material/styles';
import {
  InputBase, Button, AppBar, Toolbar, Avatar, Box, Typography,
} from '@mui/material';

import SearchIcon from '@mui/icons-material/Search';

import Logo from './Logo.svg';
import UserAvatar from './UserAvatar';

const Search = styled('div')(({ theme }) => ({
  position: 'relative',
  borderRadius: theme.shape.borderRadius,
  backgroundColor: alpha(theme.palette.common.white, 0.15),
  '&:hover': {
    backgroundColor: alpha(theme.palette.common.white, 0.25),
  },
  marginLeft: 0,
  width: '46vw',
}));

const SearchIconWrapper = styled('div')(({ theme }) => ({
  padding: theme.spacing(0, 2),
  height: '100%',
  position: 'absolute',
  pointerEvents: 'none',
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
}));

const StyledInputBase = styled(InputBase)(({ theme }) => ({
  color: 'inherit',
  '& .MuiInputBase-input': {
    padding: theme.spacing(1, 1, 1, 0),
    // vertical padding + font size from searchIcon
    paddingLeft: `calc(1em + ${theme.spacing(4)})`,
    transition: theme.transitions.create('width'),
    width: '100%',
  },
}));

export default function Layout({ children }) {
  const navigate = useNavigate();
  const [input, setInput] = useState('');
  const userID = sessionStorage.getItem('userID');

  return (
    <Box sx={{ background: '#FFF5E1', minHeight: '100vh' }}>
      <AppBar position="sticky">
        <Toolbar color="secondary">
          <Link to="/">
            <Avatar src={Logo} alt="Logo" sx={{ width: 50, height: 50 }} />
          </Link>

          <Box sx={{ width: { sm: 0, xs: 20 } }} />

          <Typography
            variant="h5"
            component="div"
            sx={{ marginLeft: '10px', flexGrow: 1, display: { xs: 'none', sm: 'block' } }}
          >
            PIE
          </Typography>

          <Search>
            <SearchIconWrapper>
              <SearchIcon style={{ fill: '#9c774e' }} />
            </SearchIconWrapper>
            <StyledInputBase
              id="inputBar"
              placeholder="Search"
              inputProps={{ 'aria-label': 'search' }}
              onChange={(e) => setInput(e.target.value)}
              onKeyDown={
                (e) => {
                  if (e.key === 'Enter' && input) {
                    document.getElementById('inputBar').value = '';
                    setInput('');
                  }
                }
              }
            />
          </Search>

          <Box sx={{ flexGrow: 1 }} />

          {userID
            ? <UserAvatar />
            : (
              <Box>
                <Button
                  variant="outlined"
                  color="secondary"
                  align="center"
                  onClick={() => { navigate('/login'); }}
                  sx={{
                    height: '40px',
                    width: '100px',
                    fontSize: 20,
                    background: '#ffffff',
                    border: '3px solid #9D2933',
                    borderRadius: '30px',
                    '&:hover': {
                      background: '#9D2933',
                      color: '#ffffff',
                      border: '3px solid #9D2933',
                    },
                  }}
                >
                  Login
                </Button>
              </Box>
            )}

        </Toolbar>
      </AppBar>

      <Box>
        {children}
      </Box>
    </Box>
  );
}
