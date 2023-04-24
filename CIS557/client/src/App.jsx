import './App.css';
import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import { createTheme, ThemeProvider } from '@mui/material';

import Home from './pages/Home';
import Login from './pages/Login/Login';
import ForgetPassword from './pages/Login/ForgetPassword';
import Register from './pages/Register';
import PublicPost from './pages/PublicGroup/PublicPost';
import PublicGroup from './pages/PublicGroup/PublicGroup';
import WritePublicPost from './pages/PublicGroup/WritePost';
import PublicGroupAdmin from './pages/PublicGroup/Admin';
import UserPage from './pages/UserPage/UserPage';
import ChangePassword from './pages/UserPage/ChangePassword';
import Deactivate from './pages/UserPage/Deactivate';
import CreateGroup from './pages/UserPage/CreateGroup';
import Notification from './pages/UserPage/Notification';

import PrivatePost from './pages/PrivateGroup/PrivatePost';
import PrivateGroup from './pages/PrivateGroup/PrivateGroup';
import WritePrivatePost from './pages/PrivateGroup/WritePost';
import PrivateGroupAdmin from './pages/PrivateGroup/Admin';

import Chat from './pages/UserPage/Chat';

import WrongPage from './pages/Page404';
import PermissionDenied from './pages/PermissionDenied';

const theme = createTheme({
  palette: {
    primary: {
      main: '#F5AF64',
    },
    secondary: {
      main: '#9D2933',
    },
  },
  typography: {
    fontFamily: 'Quicksand',
    fondWeightLight: 400,
    fondWeightRegular: 500,
    fondWeightMedium: 600,
    fondWeightBold: 700,
  },
});

function App() {
  return (
    <ThemeProvider theme={theme}>
      <Router>
        <Routes>
          <Route path="/" element={<Home />} />
          <Route path="/login" element={<Login />} />
          <Route path="/forget" element={<ForgetPassword />} />
          <Route path="/register" element={<Register />} />
          <Route path="/publicpost/:postID" element={<PublicPost />} />
          <Route path="/publicgroup/:groupID" element={<PublicGroup />} />
          <Route path="/createpublicpost/:groupID" element={<WritePublicPost />} />
          <Route path="/publicgroupadmin/:groupID" element={<PublicGroupAdmin />} />
          <Route path="/userpage" element={<UserPage />} />
          <Route path="/security/password" element={<ChangePassword />} />
          <Route path="/security/deactivate" element={<Deactivate />} />
          <Route path="/creategroup" element={<CreateGroup />} />
          <Route path="notification" element={<Notification />} />

          <Route path="/privatepost/:postID" element={<PrivatePost />} />
          <Route path="/privategroup/:groupID" element={<PrivateGroup />} />
          <Route path="/createprivatepost/:groupID" element={<WritePrivatePost />} />
          <Route path="/privategroupadmin/:groupID" element={<PrivateGroupAdmin />} />

          <Route path="/chat" element={<Chat />} />

          <Route path="/wrongpage" element={<WrongPage />} />
          <Route path="/permission" element={<PermissionDenied />} />
        </Routes>
      </Router>
    </ThemeProvider>
  );
}

export default App;
