import { makeStyles } from '@mui/styles';
import { Container, Paper, Typography } from '@mui/material';

const useStyles = makeStyles((theme) => ({
  root: {
    display: 'inlined',
  },

  paper: {
    display: 'inlined',
    textAlign: 'center',
    width: 850,
    height: 150,
    marginLeft: 'auto',
    marginRight: 'auto',
    marginTop: '10vw',
    paddingTop: 100,
    paddingBottom: 100,
  },
  padding: {
    height: '20vw',
  },

}));

export default function PermissionDenied() {
  const classes = useStyles();

  return (

    <Container className={classes.root}>
      <Paper className={classes.paper} elevation={3}>
        <Typography
          variant="h4"
        >
          You do not have permission to access this page.
        </Typography>

      </Paper>
      <div className={classes.padding} />
    </Container>

  );
}
